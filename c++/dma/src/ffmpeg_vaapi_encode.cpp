// src/ffmpeg_vaapi_encode.cpp
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
}
#include <stdio.h>

// Create av_hwdevice_ctx for VAAPI using render node (e.g., "/dev/dri/renderD128")
AVBufferRef* create_vaapi_hwdevice(const char* device_path) {
    AVBufferRef* hw_device_ctx = NULL;
    int err = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, (void*)device_path, NULL, 0);
    if (err < 0) {
        fprintf(stderr,"av_hwdevice_ctx_create failed: %d\n", err);
        return NULL;
    }
    return hw_device_ctx;
}

// Create encoder context for h264_vaapi
AVCodecContext* create_vaapi_encoder(AVBufferRef* hw_device_ctx, int width, int height, int bitrate) {
    const AVCodec* codec = avcodec_find_encoder_by_name("h264_vaapi");
    if (!codec) { fprintf(stderr,"can't find h264_vaapi\n"); return NULL; }

    AVCodecContext* enc = avcodec_alloc_context3(codec);
    enc->width = width;
    enc->height = height;
    enc->time_base = AVRational{1, 25};
    enc->framerate = AVRational{25,1};
    enc->pix_fmt = AV_PIX_FMT_VAAPI; // very important
    enc->bit_rate = bitrate;
    enc->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    if (avcodec_open2(enc, codec, NULL) < 0) {
        fprintf(stderr,"avcodec_open2 failed\n");
        avcodec_free_context(&enc);
        return NULL;
    }
    return enc;
}

/*
 * IMPORTANT:
 * The step "把 libva 的 VASurface 变为 AVFrame 可被 avcodec_send_frame 接收" 在 FFmpeg 各版本差异较大。
 * 常见方式（视 FFmpeg 版本）：
 *  A) 通过 av_hwframe_ctx_alloc/av_hwframe_transfer_data 或 av_hwframe_get_buffer 去创建 AVFrame，
 *     然后把 VASurface 直接置到 AVFrame->data[0]（低层实现需 support import via PRIME fd）。
 *  B) 如果 FFmpeg 支持从 PRIME fd 直接 import 到 AVHWFramesContext（av_hwframe_ctx_init 时设置），
 *     则可以先把 prime fd import 到 hwframe pool，再 av_frame_get_buffer 得到 AVFrame 引用。
 *
 * 请参考 FFmpeg 自带示例 doc/examples/vaapi_encode.c，这里不做一刀切的实现（因为 n8.0 的头文件/实现细节需要在目标机器上微调）。
 * 我在 README 中会给出具体调试建议。
 */

int encode_vaapi_frame(AVCodecContext* enc_ctx, AVFrame* frame, FILE* out_fp) {
    if (!enc_ctx || !frame) return -1;
    int ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) { fprintf(stderr,"avcodec_send_frame failed %d\n", ret); return -1; }
    AVPacket pkt;
    av_init_packet(&pkt);
    while (avcodec_receive_packet(enc_ctx, &pkt) == 0) {
        fwrite(pkt.data, 1, pkt.size, out_fp);
        av_packet_unref(&pkt);
    }
    return 0;
}
