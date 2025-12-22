extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_drm.h>
#include <libavutil/pixdesc.h>
}

/**
 * 创建并配置一个 AVDRMFrameDescriptor 来描述 RGBA 格式的 DMA-BUF。
 * 这是将 Vulkan 导出的图像数据传递给 FFmpeg 编码器的核心数据结构。
 *
 * @param dmabuf_fd    Vulkan 导出的 DMA-BUF 文件描述符
 * @param width        图像的宽度（像素）
 * @param height       图像的高度（像素）
 * @return             成功返回配置好的 AVDRMFrameDescriptor 指针，失败返回 NULL
 */
AVDRMFrameDescriptor* create_rgba_drm_descriptor(int dmabuf_fd, int width, int height) {
    // 1. 分配并初始化 DRM 描述符内存
    AVDRMFrameDescriptor* drm_desc = (AVDRMFrameDescriptor*)av_mallocz(sizeof(AVDRMFrameDescriptor));
    if (!drm_desc) {
        av_log(NULL, AV_LOG_ERROR, "无法分配 AVDRMFrameDescriptor 内存\n");
        return nullptr;
    }

    // 2. 描述 DMA-BUF 对象本身（即那块内存）
    drm_desc->nb_objects = 1;
    drm_desc->objects[0].fd = dmabuf_fd;       // 传入的文件描述符
    drm_desc->objects[0].size = width * height * 4; // RGBA 缓冲区大小：宽 * 高 * 4字节
    drm_desc->objects[0].format_modifier = DRM_FORMAT_MOD_LINEAR; // 假定为线性布局

    // 3. 描述图像的图层和平面（RGBA 通常为单图层、单平面）
    drm_desc->nb_layers = 1;
    drm_desc->layers[0].format = DRM_FORMAT_ABGR8888; // 使用合适的 DRM FourCC 码表示 RGBA
    drm_desc->layers[0].nb_planes = 1;

    // 3.1 配置第一个也是唯一一个平面
    drm_desc->layers[0].planes[0].object_index = 0;  // 使用上面定义的第一个（索引0）对象
    drm_desc->layers[0].planes[0].offset = 0;        // 数据从缓冲区开头开始
    drm_desc->layers[0].planes[0].pitch = width * 4; // 一行有 width 个像素，每个像素4字节

    return drm_desc;
}

/**
 * 将 Vulkan 导出的 DMA-BUF (RGBA) 转换为 FFmpeg 的 AVFrame。
 * 此函数是实现零拷贝硬件加速流水线的关键，避免了大内存的拷贝。
 *
 * @param dmabuf_fd    Vulkan 导出的 DMA-BUF 文件描述符
 * @param width        图像宽度
 * @param height       图像高度
 * @return             成功返回配置好的 AVFrame 指针，失败返回 NULL
 */
AVFrame* create_frame_from_rgba_dmabuf(int dmabuf_fd, int width, int height) {
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        av_log(NULL, AV_LOG_ERROR, "无法分配 AVFrame\n");
        close(dmabuf_fd); // 分配失败，记得关闭 fd 避免泄漏
        return nullptr;
    }

    // 1. 创建 RGBA 格式的 DRM 描述符
    AVDRMFrameDescriptor* drm_desc = create_rgba_drm_descriptor(dmabuf_fd, width, height);
    if (!drm_desc) {
        av_frame_free(&frame);
        return nullptr;
    }

    // 2. 配置 AVFrame 的基本属性
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_DRM_PRIME; // 关键：指明帧数据来自 DRM PRIME
    frame->data[0] = (uint8_t*)drm_desc;  // 将 DRM 描述符存入 AVFrame 的 data[0]

    // 3. 设置 linesize（步长）。对于 DRM PRIME，通常使用 DRM 描述符中的 pitch。
    //    这里也可以设置为 0，编码器会从 drm_desc 中读取。
    frame->linesize[0] = drm_desc->layers[0].planes[0].pitch;

    // 4. 为 DRM 描述符创建一个 AVBufferRef，用于自动管理其生命周期（引用计数）。
    frame->buf[0] = av_buffer_create((uint8_t*)drm_desc, sizeof(*drm_desc),
                                     [](void* opaque, uint8_t* data) {
                                         // 释放回调：当 AVFrame 被销毁时，自动释放 DRM 描述符并关闭 fd。
                                         AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)data;
                                         if (desc && desc->nb_objects > 0) {
                                             close(desc->objects[0].fd);
                                         }
                                         av_free(desc);
                                     }, nullptr, 0);
    if (!frame->buf[0]) {
        av_log(NULL, AV_LOG_ERROR, "无法为 DRM 描述符创建缓冲区引用\n");
        av_free(drm_desc);
        av_frame_free(&frame);
        return nullptr;
    }

    // 5. 设置时间戳（PTS）和图片类型，这对 WebRTC 等实时流至关重要。
    frame->pts = av_gettime() / 1000; // 使用相对时间戳
    frame->pict_type = AV_PICTURE_TYPE_NONE;

    av_log(NULL, AV_LOG_DEBUG, "成功从 RGBA DMA-BUF 创建 AVFrame: %dx%d, fd=%d\n", width, height, dmabuf_fd);
    return frame;
}

/**
 * WebRTC 实时编码流水线中使用 RGBA 帧的示例片段
 */
void webrtc_encode_rgba_example(AVCodecContext* encoder_ctx, int vulkan_dmabuf_fd, int width, int height) {
    AVPacket* pkt = av_packet_alloc();
    int ret;

    // 1. 将 DMA-BUF 转换为 AVFrame（零拷贝）
    AVFrame* rgba_frame = create_frame_from_rgba_dmabuf(vulkan_dmabuf_fd, width, height);
    if (!rgba_frame) {
        av_log(NULL, AV_LOG_ERROR, "无法从 DMA-BUF 创建 RGBA 帧\n");
        goto end;
    }

    // 2. 发送帧到编码器（内部可能进行硬件加速编码）
    ret = avcodec_send_frame(encoder_ctx, rgba_frame);
    if (ret < 0 && ret != AVERROR(EAGAIN)) {
        av_log(NULL, AV_LOG_ERROR, "发送帧到编码器失败: %s\n", av_err2str(ret));
        goto end;
    }

    // 3. 接收编码后的数据包
    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "编码错误: %s\n", av_err2str(ret));
            break;
        }

        // 4. 将编码后的包（pkt）通过 WebRTC 传输
        // send_to_webrtc_peer(pkt->data, pkt->size);
        av_packet_unref(pkt);
    }

end:
    av_frame_free(&rgba_frame);
    av_packet_free(&pkt);
}