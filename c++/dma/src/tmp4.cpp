extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_drm.h>
#include <libavutil/pixdesc.h>
}

/**
 * 方案一：将 Vulkan DMA-BUF fd 封装为独立的 AVFrame
 * 适合 WebRTC 实时视频中动态帧率场景
 */
class VulkanDmaBufFrameImporter {
private:
    AVBufferRef* hw_device_ctx_ = nullptr;
    AVBufferRef* hw_frames_ctx_ = nullptr;

public:
    /**
     * 初始化 DRM 硬件设备上下文
     */
    bool initialize_drm_context() {
        int ret = av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_DRM,
                                       nullptr, nullptr, 0);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "无法创建DRM硬件设备上下文: %s\n", av_err2str(ret));
            return false;
        }
        return true;
    }

    /**
     * 创建硬件帧上下文 - 专为 WebRTC 低延迟优化
     */
    bool create_hw_frames_context(int width, int height) {
        hw_frames_ctx_ = av_hwframe_ctx_alloc(hw_device_ctx_);
        if (!hw_frames_ctx_) {
            return false;
        }

        AVHWFramesContext* frames_ctx = (AVHWFramesContext*)hw_frames_ctx_->data;
        frames_ctx->format = AV_PIX_FMT_DRM_PRIME;
        frames_ctx->sw_format = AV_PIX_FMT_NV12;  // WebRTC 常用格式
        frames_ctx->width = width;
        frames_ctx->height = height;
        frames_ctx->initial_pool_size = 0;  // 重要：不预分配，使用外部fd

        // WebRTC 实时优化：禁用帧池自动扩展
        frames_ctx->flags |= AV_HWFRAME_FLAG_READONLY;

        int ret = av_hwframe_ctx_init(hw_frames_ctx_);
        if (ret < 0) {
            av_buffer_unref(&hw_frames_ctx_);
            return false;
        }

        return true;
    }

    /**
     * 将 Vulkan DMA-BUF fd 转换为可编码的 AVFrame（核心函数）
     */
    AVFrame* create_frame_from_dmabuf(int dmabuf_fd, int width, int height,
                                     AVPixelFormat sw_format = AV_PIX_FMT_NV12) {
        // 1. 创建并配置 DRM 描述符
        AVDRMFrameDescriptor* drm_desc = create_drm_descriptor(dmabuf_fd, width, height, sw_format);
        if (!drm_desc) {
            close(dmabuf_fd);  // 失败时关闭 fd
            return nullptr;
        }

        // 2. 创建 AVFrame 并关联 DRM 描述符
        AVFrame* frame = av_frame_alloc();
        if (!frame) {
            av_free(drm_desc);
            close(dmabuf_fd);
            return nullptr;
        }

        frame->width = width;
        frame->height = height;
        frame->format = AV_PIX_FMT_DRM_PRIME;
        frame->data[0] = (uint8_t*)drm_desc;  // 关键：存储 DRM 描述符
        frame->buf[0] = create_drm_buffer_ref(drm_desc);

        if (!frame->buf[0]) {
            av_frame_free(&frame);
            av_free(drm_desc);
            return nullptr;
        }

        // 3. WebRTC 时间戳优化
        frame->pts = av_gettime() / 1000;  // 相对时间戳，避免溢出
        frame->pict_type = AV_PICTURE_TYPE_NONE;

        return frame;
    }

    /**
     * 完整的 WebRTC 编码流水线示例
     */
    bool encode_webrtc_stream(int vulkan_dmabuf_fd, int width, int height) {
        // 1. 创建编码器（WebRTC 优化的 H.264）
        AVCodecContext* encoder_ctx = create_webrtc_encoder(width, height);
        if (!encoder_ctx) return false;

        // 2. 将 DMA-BUF fd 转换为 AVFrame
        AVFrame* frame = create_frame_from_dmabuf(vulkan_dmabuf_fd, width, height);
        if (!frame) {
            avcodec_free_context(&encoder_ctx);
            return false;
        }

        // 3. 编码帧（WebRTC 低延迟模式）
        AVPacket* pkt = av_packet_alloc();
        int ret = avcodec_send_frame(encoder_ctx, frame);

        if (ret >= 0) {
            ret = avcodec_receive_packet(encoder_ctx, pkt);
            if (ret >= 0) {
                // 4. 发送到 WebRTC 网络传输
                // send_to_webrtc_peer(pkt->data, pkt->size);
                av_packet_unref(pkt);
            }
        }

        // 5. 资源清理
        av_frame_free(&frame);
        av_packet_free(&pkt);
        avcodec_free_context(&encoder_ctx);

        return ret >= 0;
    }

private:
    /**
     * 创建 DRM 描述符 - 描述 DMA-BUF 的内存布局
     */
    AVDRMFrameDescriptor* create_drm_descriptor(int fd, int width, int height, AVPixelFormat fmt) {
        AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)av_mallocz(sizeof(AVDRMFrameDescriptor));
        if (!desc) return nullptr;

        desc->nb_objects = 1;
        desc->objects[0].fd = fd;
        desc->objects[0].size = width * height * 3 / 2;  // NV12 大小
        desc->objects[0].format_modifier = DRM_FORMAT_MOD_LINEAR;

        desc->nb_layers = 1;
        desc->layers[0].format = DRM_FORMAT_NV12;
        desc->layers[0].nb_planes = 2;

        // Y 平面
        desc->layers[0].planes[0].object_index = 0;
        desc->layers[0].planes[0].offset = 0;
        desc->layers[0].planes[0].pitch = width;

        // UV 平面
        desc->layers[0].planes[1].object_index = 0;
        desc->layers[0].planes[1].offset = width * height;
        desc->layers[0].planes[1].pitch = width;

        return desc;
    }

    /**
     * 创建带自定义释放回调的缓冲区引用
     */
    AVBufferRef* create_drm_buffer_ref(AVDRMFrameDescriptor* desc) {
        return av_buffer_create((uint8_t*)desc, sizeof(*desc),
                              [](void* opaque, uint8_t* data) {
                                  AVDRMFrameDescriptor* d = (AVDRMFrameDescriptor*)data;
                                  if (d && d->nb_objects > 0) {
                                      close(d->objects[0].fd);  // 重要：关闭 fd
                                  }
                                  av_free(d);
                              }, nullptr, 0);
    }

    /**
     * 创建 WebRTC 优化的编码器
     */
    AVCodecContext* create_webrtc_encoder(int width, int height) {
        const AVCodec* codec = avcodec_find_encoder_by_name("h264_v4l2m2m");
        if (!codec) return nullptr;

        AVCodecContext* ctx = avcodec_alloc_context3(codec);
        if (!ctx) return nullptr;

        // WebRTC 实时编码配置
        ctx->width = width;
        ctx->height = height;
        ctx->time_base = {1, 90000};  // RTP 时间基
        ctx->framerate = {30, 1};
        ctx->gop_size = 30;
        ctx->max_b_frames = 0;  // 无 B 帧，降低延迟
        ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;

        if (avcodec_open2(ctx, codec, nullptr) < 0) {
            avcodec_free_context(&ctx);
            return nullptr;
        }

        return ctx;
    }
};
