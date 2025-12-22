/**
 * 方案二：在创建硬件帧上下文时预配置包含 DMA-BUF fd 的帧池
 * 适合 WebRTC 实时视频中持续高帧率场景
 */
class VulkanDmaBufPoolImporter {
private:
    AVBufferRef* hw_frames_ctx_ = nullptr;
    std::vector<int> dmabuf_fds_;  // 保存多个 fd 用于帧池

public:
    /**
     * 创建包含预配置 DMA-BUF fd 的硬件帧上下文
     * 适合 WebRTC 连续编码场景，减少运行时分配
     */
    bool create_hw_frames_context_with_pool(const std::vector<int>& fds,
                                          int width, int height) {
        if (fds.empty()) {
            return false;
        }

        // 保存 fd 引用
        dmabuf_fds_ = fds;

        // 1. 创建 DRM 设备上下文
        AVBufferRef* hw_device_ctx = nullptr;
        int ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_DRM,
                                        nullptr, nullptr, 0);
        if (ret < 0) {
            return false;
        }

        // 2. 分配硬件帧上下文
        hw_frames_ctx_ = av_hwframe_ctx_alloc(hw_device_ctx);
        av_buffer_unref(&hw_device_ctx);  // 释放设备引用

        if (!hw_frames_ctx_) {
            return false;
        }

        // 3. 配置帧上下文
        AVHWFramesContext* frames_ctx = (AVHWFramesContext*)hw_frames_ctx_->data;
        frames_ctx->format = AV_PIX_FMT_DRM_PRIME;
        frames_ctx->sw_format = AV_PIX_FMT_NV12;
        frames_ctx->width = width;
        frames_ctx->height = height;
        frames_ctx->initial_pool_size = fds.size();  // 重要：设置池大小

        // 4. WebRTC 优化：设置私有数据保存 fd 信息
        frames_ctx->user_opaque = this;

        // 5. 使用自定义分配器来使用现有的 DMA-BUF fd
        frames_ctx->free = custom_pool_free;
        frames_ctx->pool = create_custom_frame_pool(fds, width, height);

        if (!frames_ctx->pool) {
            av_buffer_unref(&hw_frames_ctx_);
            return false;
        }

        ret = av_hwframe_ctx_init(hw_frames_ctx_);
        if (ret < 0) {
            av_buffer_unref(&hw_frames_ctx_);
            return false;
        }

        return true;
    }

    /**
     * 从预配置的帧池中获取帧（零分配）
     */
    AVFrame* get_frame_from_pool(int index) {
        if (index < 0 || index >= dmabuf_fds_.size()) {
            return nullptr;
        }

        AVFrame* frame = av_frame_alloc();
        if (!frame) return nullptr;

        // 使用池中预分配的帧
        int ret = av_hwframe_get_buffer(hw_frames_ctx_, frame, 0);
        if (ret < 0) {
            av_frame_free(&frame);
            return nullptr;
        }

        // 关联特定的 DMA-BUF fd
        associate_dmabuf_with_frame(frame, dmabuf_fds_[index]);

        return frame;
    }

private:
    /**
     * 创建自定义帧池使用现有的 DMA-BUF fd
     */
    AVBufferPool* create_custom_frame_pool(const std::vector<int>& fds, int width, int height) {
        // 创建自定义分配函数，使用现有的 fd 而不是分配新内存
        auto allocator = [](void* opaque, int size) -> AVBufferRef* {
            VulkanDmaBufPoolImporter* importer = (VulkanDmaBufPoolImporter*)opaque;
            static int current_fd_index = 0;

            if (current_fd_index >= importer->dmabuf_fds_.size()) {
                return nullptr;
            }

            int fd = importer->dmabuf_fds_[current_fd_index++];
            return create_buffer_from_existing_fd(fd, width, height);
        };

        return av_buffer_pool_init2(sizeof(AVDRMFrameDescriptor), this, allocator, custom_pool_free);
    }

    /**
     * 从现有 fd 创建 AVBufferRef
     */
    static AVBufferRef* create_buffer_from_existing_fd(int fd, int width, int height) {
        AVDRMFrameDescriptor* desc = create_drm_descriptor(fd, width, height, AV_PIX_FMT_NV12);
        if (!desc) return nullptr;

        return av_buffer_create((uint8_t*)desc, sizeof(*desc),
                               [](void* opaque, uint8_t* data) {
                                   AVDRMFrameDescriptor* d = (AVDRMFrameDescriptor*)data;
                                   if (d && d->nb_objects > 0) {
                                       close(d->objects[0].fd);
                                   }
                                   av_free(d);
                               }, nullptr, 0);
    }

    /**
     * 自定义池释放函数
     */
    static void custom_pool_free(void* opaque, uint8_t* data) {
        // 这里可以实现自定义的清理逻辑
        // 注意：DMA-BUF fd 的生命周期由外部管理
    }

    /**
     * 将 DMA-BUF fd 与 AVFrame 关联
     */
    void associate_dmabuf_with_frame(AVFrame* frame, int fd) {
        // 这里需要根据具体的 FFmpeg 版本和硬件支持来实现
        // 可能涉及设置 frame->data[0] 或使用其他扩展机制
    }
};