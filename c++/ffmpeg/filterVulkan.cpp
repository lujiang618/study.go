// ff_vulkan_realtime_rgba_to_nv12_h264_vulkan.c
// Realtime zero-copy example: Vulkan-rendered RGBA -> FFmpeg filtergraph (hwupload + scale_vulkan)
// convert RGBA -> NV12 on GPU, then encode with h264_vulkan. Encoded AVPackets are handed
// to a user-provided callback (e.g. to push over WebRTC). No file output, no WebRTC stack.
//
// Key design points:
// - The renderer produces RGBA pixel data (this example simulates the data). In a real app
//   you should prefer GPU-side handoff (external Vulkan image import / dmabuf) rather than
//   staging via CPU memory when possible. This example **accepts** RGBA software frames and
//   attempts to let the filtergraph `hwupload` upload them into the Vulkan hwcontext — the
//   success of that operation depends on your FFmpeg build and GPU driver.
// - Filtergraph: buffersrc (RGBA sw frame) -> hwupload=derive_device=vulkan ->
//   scale_vulkan=format=nv12 -> buffersink (request AV_PIX_FMT_VULKAN frames) -> encoder
// - Encoder: h264_vulkan expects hw frames in AV_PIX_FMT_VULKAN. We attach nothing special
//   to encoder; the frames coming from the filtergraph determine the hw device context usage.
// - Output: instead of writing to file we call a callback send_packet_cb(opaque, &pkt)
//   with each encoded packet. It's the caller's responsibility to send over the network.
//
// Build: link with libavfilter, libavcodec, libavformat, libavutil
// Example (may need adjustments depending on your pkg-config names):
// gcc -o ff_vulkan_realtime ff_vulkan_realtime_rgba_to_nv12_h264_vulkan.c \
//   `pkg-config --cflags --libs libavcodec libavformat libavfilter libavutil` -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#define CHECK_ERR(ret, msg)                         \
    if ((ret) < 0)                                  \
    {                                               \
        char errbuf[128];                           \
        av_strerror((ret), errbuf, sizeof(errbuf)); \
        fprintf(stderr, "%s: %s", msg, errbuf);     \
        goto fail;                                  \
    }

static const int WIDTH  = 1280;
static const int HEIGHT = 720;
static const int FPS    = 30;

// User callback type: deliver encoded packet to network layer (WebRTC, etc.)
typedef int (*send_packet_cb_t)(void* opaque, AVPacket* pkt);

// Simple demo callback that prints packet info (replace with real network send)
static int demo_send_packet(void* opaque, AVPacket* pkt)
{
    (void)opaque;
    fprintf(stderr, "[demo] packet pts=%" PRId64 " size=%d flags=0x%x", pkt->pts, pkt->size, pkt->flags);
    return 0;
}

// Main real-time pipeline struct
typedef struct RealtimeEncoder
{
    AVBufferRef*     hw_device_ctx;
    AVFilterGraph*   filter_graph;
    AVFilterContext* buffersrc_ctx;
    AVFilterContext* buffersink_ctx;
    AVCodecContext*  enc_ctx;
    send_packet_cb_t send_cb;
    void*            send_opaque;
} RealtimeEncoder;

// Initialize pipeline: create vulkan hw device, build filtergraph and open encoder
int realtime_init(RealtimeEncoder* r, send_packet_cb_t cb, void* opaque)
{
    int ret        = 0;
    r->send_cb     = cb;
    r->send_opaque = opaque;

    // create Vulkan device
    ret = av_hwdevice_ctx_create(&r->hw_device_ctx, AV_HWDEVICE_TYPE_VULKAN, NULL, NULL, 0);
    CHECK_ERR(ret, "av_hwdevice_ctx_create");
    fprintf(stderr, "[init] Vulkan hwdevice created");

    // build filter graph
    r->filter_graph            = avfilter_graph_alloc();
    const AVFilter* buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");
    if (!buffersrc || !buffersink)
    {
        fprintf(stderr, "filter not found");
        return AVERROR_FILTER_NOT_FOUND;
    }

    // buffer source args for RGBA
    char args[512];
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=1/1", WIDTH, HEIGHT, AV_PIX_FMT_RGBA, 1, FPS);
    ret = avfilter_graph_create_filter(&r->buffersrc_ctx, buffersrc, "in", args, NULL, r->filter_graph);
    CHECK_ERR(ret, "create buffer source");

    ret = avfilter_graph_create_filter(&r->buffersink_ctx, buffersink, "out", NULL, NULL, r->filter_graph);
    CHECK_ERR(ret, "create buffer sink");

    // request Vulkan frames out of the filterchain
    AVPixelFormat sink_pix_fmts[] = {AV_PIX_FMT_VULKAN, AV_PIX_FMT_NONE};
    ret                           = av_opt_set_int_list(r->buffersink_ctx, "pix_fmts", sink_pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    CHECK_ERR(ret, "set buffersink pix_fmts");

    // filter description: upload to vulkan then convert to NV12 on GPU
    const char* filter_descr = "hwupload=derive_device=vulkan,scale_vulkan=w=1280:h=720:format=nv12";

    AVFilterInOut* inputs  = avfilter_inout_alloc();
    AVFilterInOut* outputs = avfilter_inout_alloc();
    outputs->name          = av_strdup("in");
    outputs->filter_ctx    = r->buffersrc_ctx;
    outputs->pad_idx       = 0;
    outputs->next          = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = r->buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    ret = avfilter_graph_parse_ptr(r->filter_graph, filter_descr, &inputs, &outputs, NULL);
    CHECK_ERR(ret, "parse filter graph");
    ret = avfilter_graph_config(r->filter_graph, NULL);
    CHECK_ERR(ret, "config filter graph");
    fprintf(stderr, "[init] filter graph configured");

    // open encoder
    AVCodec* enc = avcodec_find_encoder_by_name("h264_vulkan");
    if (!enc)
    {
        fprintf(stderr, "h264_vulkan not available");
        return AVERROR_ENCODER_NOT_FOUND;
    }
    r->enc_ctx = avcodec_alloc_context3(enc);
    if (!r->enc_ctx)
        return AVERROR(ENOMEM);
    r->enc_ctx->width     = WIDTH;
    r->enc_ctx->height    = HEIGHT;
    r->enc_ctx->time_base = (AVRational){1, FPS};
    r->enc_ctx->framerate = (AVRational){FPS, 1};
    r->enc_ctx->pix_fmt   = AV_PIX_FMT_VULKAN; // encoder expects hw frames
    r->enc_ctx->bit_rate  = 2000000;
    r->enc_ctx->gop_size  = 12;

    AVDictionary* opts = NULL;
    av_dict_set(&opts, "preset", "veryfast", 0);
    ret = avcodec_open2(r->enc_ctx, enc, &opts);
    CHECK_ERR(ret, "open encoder");
    fprintf(stderr, "[init] encoder opened (h264_vulkan)");

    av_freep(&inputs->name);
    av_freep(&outputs->name);
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return 0;

fail:
    return ret;
}

// Shutdown and free
void realtime_shutdown(RealtimeEncoder* r)
{
    if (!r)
        return;
    if (r->enc_ctx)
        avcodec_free_context(&r->enc_ctx);
    if (r->filter_graph)
        avfilter_graph_free(&r->filter_graph);
    if (r->hw_device_ctx)
        av_buffer_unref(&r->hw_device_ctx);
}

// Push one RGBA frame (CPU memory) into pipeline. In real-world, prefer GPU-side handoff.
int realtime_push_rgba(RealtimeEncoder* r, uint8_t* rgba_data, int linesize, int64_t pts)
{
    int      ret = 0;
    AVFrame* src = av_frame_alloc();
    if (!src)
        return AVERROR(ENOMEM);
    src->format = AV_PIX_FMT_RGBA;
    src->width  = WIDTH;
    src->height = HEIGHT;
    // attach existing buffer without copy using av_image_fill_arrays (we still reference CPU memory)
    ret = av_image_fill_arrays(src->data, src->linesize, rgba_data, AV_PIX_FMT_RGBA, WIDTH, HEIGHT, 1);
    if (ret < 0)
    {
        av_frame_free(&src);
        return ret;
    }
    src->pts = pts;

    // push to buffersrc
    ret = av_buffersrc_add_frame_flags(r->buffersrc_ctx, src, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0)
    {
        av_frame_free(&src);
        return ret;
    }

    // pull output frame (Vulkan-backed) from buffersink
    AVFrame* filt_frame = av_frame_alloc();
    ret                 = av_buffersink_get_frame(r->buffersink_ctx, filt_frame);
    if (ret == AVERROR(EAGAIN))
    {
        av_frame_free(&filt_frame);
        return 0;
    }
    if (ret < 0)
    {
        av_frame_free(&filt_frame);
        return ret;
    }

    // check format
    if (filt_frame->format != AV_PIX_FMT_VULKAN)
    {
        fprintf(stderr, "[push] filtered frame not Vulkan format, got %d", filt_frame->format);
        av_frame_free(&filt_frame);
        return AVERROR_PATCHWELCOME;
    }

    // send hw frame to encoder
    ret = avcodec_send_frame(r->enc_ctx, filt_frame);
    if (ret < 0)
    {
        fprintf(stderr, "avcodec_send_frame failed: %d", ret);
    }

    // receive packets and forward via callback
    while (1)
    {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
        ret      = avcodec_receive_packet(r->enc_ctx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_packet_unref(&pkt);
            break;
        }
        if (ret < 0)
        {
            av_packet_unref(&pkt);
            break;
        }
        // deliver
        if (r->send_cb)
            r->send_cb(r->send_opaque, &pkt);
        av_packet_unref(&pkt);
    }

    av_frame_free(&filt_frame);
    // buffersrc_add_frame had KEEP_REF -> free src
    av_frame_free(&src);
    return 0;
}

// Example main: simulate RGBA frames produced by renderer and push into pipeline
int main(void)
{
    RealtimeEncoder r   = {0};
    int             ret = realtime_init(&r, demo_send_packet, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to init pipeline: %d", ret);
        return -1;
    }

    // simulate RGBA buffer (one contiguous buffer)
    uint8_t* rgba = av_malloc(WIDTH * HEIGHT * 4);
    if (!rgba)
    {
        realtime_shutdown(&r);
        return -1;
    }
    // fill gradient
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            uint8_t* p = rgba + (y * WIDTH + x) * 4;
            p[0]       = (x * 255) / WIDTH;
            p[1]       = (y * 255) / HEIGHT;
            p[2]       = 128;
            p[3]       = 255;
        }
    }

    int frames = FPS * 2;
    for (int i = 0; i < frames; ++i)
    {
        realtime_push_rgba(&r, rgba, WIDTH * 4, i);
    }

    // flush encoder
    avcodec_send_frame(r.enc_ctx, NULL);
    while (1)
    {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
        ret      = avcodec_receive_packet(r.enc_ctx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_packet_unref(&pkt);
            break;
        }
        if (ret >= 0)
        {
            demo_send_packet(NULL, &pkt);
            av_packet_unref(&pkt);
        }
        else
            break;
    }

    av_free(rgba);
    realtime_shutdown(&r);
    return 0;
}

// 在 RgbaToNv12 类内或相应 cpp 文件
int RgbaToNv12::BuildManualGraph()
{
    int ret = 0;
    const AVFilter *f_buffer = avfilter_get_by_name("buffer");
    const AVFilter *f_hwupload = avfilter_get_by_name("hwupload");
    const AVFilter *f_scalevk = avfilter_get_by_name("scale_vulkan");
    const AVFilter *f_buffersink = avfilter_get_by_name("buffersink");
    if (!f_buffer || !f_hwupload || !f_scalevk || !f_buffersink) return AVERROR_FILTER_NOT_FOUND;

    // 1) alloc filters (but not init)
    bufferSrcCtx_   = avfilter_graph_alloc_filter(filterGraph_, f_buffer, "in");
    AVFilterContext* hwuploadCtx = avfilter_graph_alloc_filter(filterGraph_, f_hwupload, "hwup");
    AVFilterContext* scaleCtx    = avfilter_graph_alloc_filter(filterGraph_, f_scalevk, "scalevk");
    bufferSlinkCtx_ = avfilter_graph_alloc_filter(filterGraph_, f_buffersink, "out");
    if (!bufferSrcCtx_ || !hwuploadCtx || !scaleCtx || !bufferSlinkCtx_) return AVERROR(ENOMEM);

    // 2) set buffer src args BEFORE init (buffer uses init string to set width/height/pix_fmt)
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=1/%d:pixel_aspect=1/1",
             width_, height_, AV_PIX_FMT_RGBA, fps_);

    // 3) set buffersink pix_fmts BEFORE init (non-runtime)
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_NV12, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(bufferSlinkCtx_, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) return ret;

    // 4) bind hw device to hwupload BEFORE init
    hwuploadCtx->hw_device_ctx = av_buffer_ref(hwDeviceCtx_);

    // 5) initialize each filter (init string for buffer; others NULL)
    ret = avfilter_init_str(bufferSrcCtx_, args);
    if (ret < 0)
        return ret;
    // hwupload init with no args
    ret = avfilter_init_str(hwuploadCtx, NULL); if (ret < 0) return ret;
    // scale_vulkan: provide format/size as init string to avoid parse later
    char scaleArgs[256];
    snprintf(scaleArgs, sizeof(scaleArgs), "w=%d:h=%d:format=nv12", width_, height_);
    ret = avfilter_init_str(scaleCtx, scaleArgs); if (ret < 0) return ret;
    // buffersink init
    ret = avfilter_init_str(bufferSlinkCtx_, NULL); if (ret < 0) return ret;

    // 6) link: buffer -> hwupload -> scale_vulkan -> buffersink
    ret = avfilter_link(bufferSrcCtx_, 0, hwuploadCtx, 0); if (ret < 0) return ret;
    ret = avfilter_link(hwuploadCtx, 0, scaleCtx, 0); if (ret < 0) return ret;
    ret = avfilter_link(scaleCtx, 0, bufferSlinkCtx_, 0); if (ret < 0) return ret;

    // 7) final config
    ret = avfilter_graph_config(filterGraph_, NULL);
    return ret;
}
