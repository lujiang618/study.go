// 1) 创建 hw device already done:
// AVBufferRef* hw_device_ctx;
// av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, "/dev/dri/renderD128", NULL, 0);

// 2) 创建并初始化 hw frames context（用于把导入的 AVFrame 与 device 关联）
AVBufferRef* hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx);
AVHWFramesContext* framesCtx = (AVHWFramesContext*)(hw_frames_ref->data);
framesCtx->format = AV_PIX_FMT_VAAPI;   // hw format
framesCtx->sw_format = AV_PIX_FMT_NV12; // encoder expected sw format (may be NV12 even if input is RGBA)
framesCtx->width = WIDTH;
framesCtx->height = HEIGHT;
framesCtx->initial_pool_size = 4; // choose per above recommendations
int ret = av_hwframe_ctx_init(hw_frames_ref);
if (ret < 0) {
    // handle error
}

// 3) 导入 dmabuf -> 创建 wrapper AVFrame pool（你的 "外部 pool"）
AVFrame* import_frame_from_dmabuf(int dmabuf_fd, int width, int height, int drm_format, int stride) {
    AVFrame* frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_VAAPI;
    frame->width  = width;
    frame->height = height;
    // IMPORTANT: associate with hw_frames_ref (not hw_device_ctx)
    frame->hw_frames_ctx = av_buffer_ref(hw_frames_ref);

    AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)av_mallocz(sizeof(*desc));
    desc->nb_objects = 1;
    desc->objects[0].fd = dmabuf_fd;
    desc->nb_layers = 1;
    desc->layers[0].format = drm_format;
    desc->layers[0].nb_planes = 1;
    desc->layers[0].planes[0].object_index = 0;
    desc->layers[0].planes[0].offset = 0;
    desc->layers[0].planes[0].pitch  = stride;

    frame->data[0] = (uint8_t*)desc;
    frame->buf[0] = av_buffer_create((uint8_t*)desc, sizeof(*desc),
        [](void* opaque, uint8_t* data){
            AVDRMFrameDescriptor* d = (AVDRMFrameDescriptor*)data;
            if (d && d->nb_objects > 0) close(d->objects[0].fd);
            av_free(d);
        }, NULL, AV_BUFFER_FLAG_READONLY);

    return frame;
}

// 4) encode loop (round-robin)
std::vector<AVFrame*> pool = create_wrappers(...); // create from duplicated dmabuf fds
size_t idx = 0;
for (;;) {
    AVFrame* frame = pool[idx];
    // ensure Vulkan finished rendering to this dmabuf (use semaphore fd or vkQueueWaitIdle)
    frame->pts = pts++;
    avcodec_send_frame(enc_ctx, frame);
    // drain packets...
    idx = (idx + 1) % pool.size();
}
