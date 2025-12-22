extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_drm.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
}

/**
 * Vulkan DMA-BUF 到 FFmpeg 硬件帧转换器
 * 专为 WebRTC 实时视频编码优化
 */
class VulkanDmaBufToFFmpeg {
private:
    AVBufferRef* hw_device_ctx_ = nullptr;
    AVCodecContext* encoder_ctx_ = nullptr;
    int drm_fd_ = -1;

public:
    /**
     * 初始化 DRM 硬件设备上下文
     * 用于处理 DMA-BUF 文件描述符
     */
    bool initialize_drm_device() {
        int ret = av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_DRM,
                                       nullptr, nullptr, 0);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "无法创建DRM硬件设备上下文: %s\n", av_err2str(ret));
            return false;
        }
        return true;
    }

    /**
     * 从 Vulkan DMA-BUF fd 创建 AVFrame
     * 核心功能：实现零拷贝硬件帧创建
     */
    AVFrame* create_frame_from_vulkan_dmabuf(int dmabuf_fd, int width, int height,
                                           AVPixelFormat sw_format = AV_PIX_FMT_NV12) {
        AVFrame* frame = av_frame_alloc();
        if (!frame) {
            av_log(nullptr, AV_LOG_ERROR, "无法分配AVFrame\n");
            close(dmabuf_fd); // 关闭fd，避免泄漏
            return nullptr;
        }

        // 配置帧参数
        frame->width = width;
        frame->height = height;
        frame->format = AV_PIX_FMT_DRM_PRIME; // 关键：使用DRM PRIME格式

        // 创建DRM描述符来描述DMA-BUF
        AVDRMFrameDescriptor* drm_desc = create_drm_descriptor(dmabuf_fd, width, height, sw_format);
        if (!drm_desc) {
            av_frame_free(&frame);
            return nullptr;
        }

        // 将DRM描述符存入AVFrame（零拷贝的关键）
        frame->data[0] = (uint8_t*)drm_desc;
        frame->buf[0] = av_buffer_create((uint8_t*)drm_desc, sizeof(*drm_desc),
                                       free_drm_descriptor, nullptr, 0);

        if (!frame->buf[0]) {
            av_log(nullptr, AV_LOG_ERROR, "无法创建DRM描述符缓冲区\n");
            av_free(drm_desc);
            av_frame_free(&frame);
            return nullptr;
        }

        // 设置时间戳（WebRTC实时优化）
        frame->pts = av_gettime() / 1000; // 使用相对时间戳
        frame->pict_type = AV_PICTURE_TYPE_NONE;

        av_log(nullptr, AV_LOG_DEBUG, "成功从Vulkan DMA-BUF创建AVFrame: %dx%d, fd=%d\n",
              width, height, dmabuf_fd);

        return frame;
    }

    /**
     * 初始化硬件编码器（H.264，适用于WebRTC）
     */
    bool initialize_encoder(int width, int height, int bitrate = 2000000) {
        // 查找支持DRM PRIME的编码器
        const AVCodec* codec = avcodec_find_encoder_by_name("h264_v4l2m2m"); // 或使用其他支持DMA-BUF的编码器
        if (!codec) {
            // 回退到软件编码器（仍可使用DMA-BUF零拷贝）
            codec = avcodec_find_encoder(AV_CODEC_ID_H264);
            if (!codec) {
                av_log(nullptr, AV_LOG_ERROR, "未找到H.264编码器\n");
                return false;
            }
        }

        encoder_ctx_ = avcodec_alloc_context3(codec);
        if (!encoder_ctx_) {
            av_log(nullptr, AV_LOG_ERROR, "无法分配编码器上下文\n");
            return false;
        }

        // WebRTC实时编码配置
        configure_encoder_for_realtime(width, height, bitrate);

        // 尝试使用DRM PRIME格式
        if (codec->pix_fmts) {
            for (const AVPixelFormat* p = codec->pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
                if (*p == AV_PIX_FMT_DRM_PRIME) {
                    encoder_ctx_->pix_fmt = AV_PIX_FMT_DRM_PRIME;
                    av_log(nullptr, AV_LOG_INFO, "编码器支持DRM PRIME格式\n");
                    break;
                }
            }
        }

        // 打开编码器
        AVDictionary* opts = nullptr;
        av_dict_set(&opts, "preset", "veryfast", 0); // WebRTC低延迟预设
        av_dict_set(&opts, "tune", "zerolatency", 0);

        int ret = avcodec_open2(encoder_ctx_, codec, &opts);
        av_dict_free(&opts);

        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "无法打开编码器: %s\n", av_err2str(ret));
            return false;
        }

        return true;
    }

    /**
     * 编码Vulkan DMA-BUF帧（零拷贝路径）
     */
    int encode_vulkan_frame(int dmabuf_fd, int width, int height, AVPacket* output_packet) {
        // 1. 从DMA-BUF fd创建AVFrame（零拷贝）
        AVFrame* frame = create_frame_from_vulkan_dmabuf(dmabuf_fd, width, height);
        if (!frame) {
            return AVERROR(ENOMEM);
        }

        // 2. 发送帧到编码器
        int ret = avcodec_send_frame(encoder_ctx_, frame);
        if (ret < 0 && ret != AVERROR(EAGAIN)) {
            av_log(nullptr, AV_LOG_ERROR, "发送帧到编码器失败: %s\n", av_err2str(ret));
            av_frame_free(&frame);
            return ret;
        }

        // 3. 接收编码后的包
        ret = avcodec_receive_packet(encoder_ctx_, output_packet);

        // 4. 释放帧（不会释放DMA-BUF内存，只释放描述符）
        av_frame_free(&frame);

        return ret;
    }

private:
    /**
     * 创建DRM描述符来描述DMA-BUF
     */
    AVDRMFrameDescriptor* create_drm_descriptor(int fd, int width, int height, AVPixelFormat sw_format) {
        AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)av_mallocz(sizeof(AVDRMFrameDescriptor));
        if (!desc) {
            return nullptr;
        }

        // 配置DRM对象（DMA-BUF）
        desc->nb_objects = 1;
        desc->objects[0].fd = fd;
        desc->objects[0].size = calculate_dmabuf_size(width, height, sw_format);
        desc->objects[0].format_modifier = DRM_FORMAT_MOD_LINEAR; // 通常为线性布局

        // 配置图层（图像平面）
        desc->nb_layers = 1;
        desc->layers[0].format = drm_format_from_pixel(sw_format);
        desc->layers[0].nb_planes = get_plane_count(sw_format);

        // 配置平面参数
        for (int i = 0; i < desc->layers[0].nb_planes; i++) {
            desc->layers[0].planes[i].object_index = 0;
            desc->layers[0].planes[i].offset = get_plane_offset(i, width, height, sw_format);
            desc->layers[0].planes[i].pitch = get_plane_pitch(i, width, sw_format);
        }

        return desc;
    }

    /**
     * 计算DMA-BUF所需的大小
     */
    size_t calculate_dmabuf_size(int width, int height, AVPixelFormat fmt) {
        switch (fmt) {
            case AV_PIX_FMT_NV12:
                return width * height * 3 / 2; // YUV420 semi-planar
            case AV_PIX_FMT_YUV420P:
                return width * height * 3 / 2; // YUV420 planar
            case AV_PIX_FMT_RGBA:
                return width * height * 4; // RGBA
            default:
                return width * height * 3; // 保守估计
        }
    }

    /**
     * 从AVPixelFormat获取DRM FourCC格式
     */
    uint32_t drm_format_from_pixel(AVPixelFormat fmt) {
        switch (fmt) {
            case AV_PIX_FMT_NV12: return DRM_FORMAT_NV12;
            case AV_PIX_FMT_YUV420P: return DRM_FORMAT_YUV420;
            case AV_PIX_FMT_RGBA: return DRM_FORMAT_ABGR8888;
            default: return DRM_FORMAT_INVALID;
        }
    }

    /**
     * 获取平面数量
     */
    int get_plane_count(AVPixelFormat fmt) {
        switch (fmt) {
            case AV_PIX_FMT_NV12: return 2; // Y和UV平面
            case AV_PIX_FMT_YUV420P: return 3; // Y、U、V平面
            case AV_PIX_FMT_RGBA: return 1; // 单平面
            default: return 1;
        }
    }

    /**
     * 获取平面偏移量
     */
    uint32_t get_plane_offset(int plane, int width, int height, AVPixelFormat fmt) {
        switch (fmt) {
            case AV_PIX_FMT_NV12:
                return (plane == 0) ? 0 : width * height; // Y平面在0，UV平面在Y之后
            case AV_PIX_FMT_YUV420P:
                if (plane == 0) return 0;
                if (plane == 1) return width * height;
                return width * height * 5 / 4; // V平面
            default:
                return 0;
        }
    }

    /**
     * 获取平面步长（pitch）
     */
    uint32_t get_plane_pitch(int plane, int width, AVPixelFormat fmt) {
        switch (fmt) {
            case AV_PIX_FMT_NV12:
                return (plane == 0) ? width : width; // Y和UV有相同的pitch
            case AV_PIX_FMT_YUV420P:
                return (plane == 0) ? width : width / 2; // UV平面宽度减半
            case AV_PIX_FMT_RGBA:
                return width * 4;
            default:
                return width;
        }
    }

    /**
     * 配置编码器用于WebRTC实时编码
     */
    void configure_encoder_for_realtime(int width, int height, int bitrate) {
        encoder_ctx_->width = width;
        encoder_ctx_->height = height;
        encoder_ctx_->time_base = {1, 90000}; // RTP时间基
        encoder_ctx_->framerate = {30, 1};    // 30fps，WebRTC常用
        encoder_ctx_->bit_rate = bitrate;
        encoder_ctx_->gop_size = 30;          // 关键帧间隔
        encoder_ctx_->max_b_frames = 0;       // 无B帧，降低延迟

        // WebRTC低延迟优化
        encoder_ctx_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        encoder_ctx_->flags2 |= AV_CODEC_FLAG2_FAST;
        encoder_ctx_->thread_count = 1;       // 单线程避免同步开销
    }

    /**
     * DRM描述符释放回调
     */
    static void free_drm_descriptor(void* opaque, uint8_t* data) {
        AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)data;
        if (desc) {
            // 关闭DMA-BUF文件描述符
            if (desc->nb_objects > 0 && desc->objects[0].fd >= 0) {
                close(desc->objects[0].fd);
            }
            av_free(desc);
        }
    }

public:
    ~VulkanDmaBufToFFmpeg() {
        if (encoder_ctx_) {
            avcodec_free_context(&encoder_ctx_);
        }
        if (hw_device_ctx_) {
            av_buffer_unref(&hw_device_ctx_);
        }
    }
};