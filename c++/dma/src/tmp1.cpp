#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_drm.h>
#include <libavutil/pixdesc.h>
}

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <X11/Xlib.h>

class VulkanVAAPIDemo {
private:
    // Vulkan相关变量
    VkInstance instance_;
    VkPhysicalDevice physical_device_;
    VkDevice device_;
    VkQueue queue_;
    VkImage vulkan_image_;
    VkDeviceMemory vulkan_memory_;
    VkFramebuffer framebuffer_;
    VkRenderPass render_pass_;
    VkCommandBuffer command_buffer_;

    // FFmpeg相关变量
    AVCodecContext* codec_ctx_;
    AVBufferRef* hw_device_ctx_;
    AVFrame* hw_frame_;

    int width_, height_;
    int dmabuf_fd_;

public:
    VulkanVAAPIDemo(int width, int height) : width_(width), height_(height), dmabuf_fd_(-1) {
        initVulkan();
        initFFmpegVAAPI();
    }

    ~VulkanVAAPIDemo() {
        cleanup();
    }

private:
    void initVulkan() {
        // 创建Vulkan实例
        VkInstanceCreateInfo instance_info = {};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        vkCreateInstance(&instance_info, nullptr, &instance_);

        // 选择物理设备
        uint32_t device_count = 1;
        vkEnumeratePhysicalDevices(instance_, &device_count, &physical_device_);

        // 创建逻辑设备
        VkDeviceCreateInfo device_info = {};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // 启用外部内存扩展
        const char* extensions[] = {VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME};
        device_info.enabledExtensionCount = 1;
        device_info.ppEnabledExtensionNames = extensions;

        vkCreateDevice(physical_device_, &device_info, nullptr, &device_);
        vkGetDeviceQueue(device_, 0, 0, &queue_);

        // 创建可导出DMA-BUF的Vulkan图像
        createExportableImage();

        // 创建渲染通道和帧缓冲区
        createRenderPass();
        createFramebuffer();

        // 创建命令缓冲区
        allocateCommandBuffer();
    }

    void createExportableImage() {
        VkImageCreateInfo image_info = {};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = VK_FORMAT_B8G8R8A8_UNORM; // 与VAAPI兼容的格式
        image_info.extent = {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1};
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling = VK_IMAGE_TILING_LINEAR; // 线性布局便于导出
        image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // 启用外部内存
        VkExternalMemoryImageCreateInfo external_info = {};
        external_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
        external_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
        image_info.pNext = &external_info;

        vkCreateImage(device_, &image_info, nullptr, &vulkan_image_);

        // 分配内存并导出为DMA-BUF
        allocateAndExportMemory();
    }

    void allocateAndExportMemory() {
        VkMemoryRequirements mem_reqs;
        vkGetImageMemoryRequirements(device_, vulkan_image_, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_reqs.size;
        alloc_info.memoryTypeIndex = findMemoryType(mem_reqs.memoryTypeBits);

        // 启用导出分配
        VkExportMemoryAllocateInfo export_alloc_info = {};
        export_alloc_info.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
        export_alloc_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
        alloc_info.pNext = &export_alloc_info;

        vkAllocateMemory(device_, &alloc_info, nullptr, &vulkan_memory_);
        vkBindImageMemory(device_, vulkan_image_, vulkan_memory_, 0);

        // 导出为DMA-BUF文件描述符
        VkMemoryGetFdInfoKHR get_fd_info = {};
        get_fd_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
        get_fd_info.memory = vulkan_memory_;
        get_fd_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

        // 获取DMA-BUF文件描述符
        auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device_, "vkGetMemoryFdKHR");
        vkGetMemoryFdKHR(device_, &get_fd_info, &dmabuf_fd_);
    }

    uint32_t findMemoryType(uint32_t type_filter) {
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) &&
                (mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
                return i;
            }
        }
        throw std::runtime_error("找不到合适的显存类型");
    }

    void initFFmpegVAAPI() {
        // 初始化VAAPI硬件设备上下文[1](@ref)
        int ret = av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_VAAPI,
                                        "/dev/dri/renderD128", NULL, 0);
        if (ret < 0) {
            throw std::runtime_error("无法创建VAAPI设备上下文");
        }

        // 查找H.264编码器
        const AVCodec* codec = avcodec_find_encoder_by_name("h264_vaapi");
        if (!codec) {
            throw std::runtime_error("找不到h264_vaapi编码器");
        }

        // 创建编码器上下文
        codec_ctx_ = avcodec_alloc_context3(codec);
        codec_ctx_->width = width_;
        codec_ctx_->height = height_;
        codec_ctx_->time_base = {1, 30};  // 30fps，适合实时编码
        codec_ctx_->framerate = {30, 1};
        codec_ctx_->pix_fmt = AV_PIX_FMT_VAAPI;
        codec_ctx_->bit_rate = 4000000;   // 4Mbps，适合1080p实时视频
        codec_ctx_->gop_size = 30;        // 关键帧间隔
        codec_ctx_->max_b_frames = 0;     // 实时编码通常禁用B帧

        // 创建硬件帧上下文
        createHWFrameContext();

        // 打开编码器
        ret = avcodec_open2(codec_ctx_, codec, NULL);
        if (ret < 0) {
            throw std::runtime_error("无法打开编码器");
        }

        // 分配硬件帧
        hw_frame_ = av_frame_alloc();
        hw_frame_->format = AV_PIX_FMT_VAAPI;
        hw_frame_->width = width_;
        hw_frame_->height = height_;
    }

    void createHWFrameContext() {
        // 创建硬件帧上下文[1](@ref)
        AVBufferRef* hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx_);
        AVHWFramesContext* frames_ctx = (AVHWFramesContext*)hw_frames_ref->data;

        frames_ctx->format = AV_PIX_FMT_VAAPI;
        frames_ctx->sw_format = AV_PIX_FMT_NV12;  // VAAPI常用格式
        frames_ctx->width = width_;
        frames_ctx->height = height_;
        frames_ctx->initial_pool_size = 3;       // 实时编码缓冲池大小

        int ret = av_hwframe_ctx_init(hw_frames_ref);
        if (ret < 0) {
            av_buffer_unref(&hw_frames_ref);
            throw std::runtime_error("无法初始化硬件帧上下文");
        }

        codec_ctx_->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
        av_buffer_unref(&hw_frames_ref);
    }

    void renderFrame() {
        // 开始渲染命令
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(command_buffer_, &begin_info);

        // 设置渲染通道
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_pass_;
        render_pass_info.framebuffer = framebuffer_;
        render_pass_info.renderArea = {{0, 0}, {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)}};

        vkCmdBeginRenderPass(command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        // 在此处添加具体的Vulkan渲染命令
        // 例如：清除屏幕、绘制三角形等

        vkCmdEndRenderPass(command_buffer_);
        vkEndCommandBuffer(command_buffer_);

        // 提交命令缓冲区
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer_;

        vkQueueSubmit(queue_, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue_);  // 等待渲染完成
    }

    void encodeFrame() {
        // 将DMA-BUF导入为VAAPI表面[3](@ref)
        VASurfaceID va_surface;
        VAStatus va_status;

        // 获取VA显示
        VADisplay va_display = vaGetDisplayDRM(dmabuf_fd_);
        va_status = vaCreateSurfaces(va_display, VA_RT_FORMAT_YUV420,
                                   width_, height_, &va_surface, 1, NULL, 0);

        if (va_status != VA_STATUS_SUCCESS) {
            throw std::runtime_error("无法创建VA表面");
        }

        // 配置硬件帧
        hw_frame_->data[3] = (uint8_t*)(uintptr_t)va_surface;  // VA表面指针
        hw_frame_->pts = av_gettime() / 1000000;  // 当前时间戳

        // 发送帧进行编码
        int ret = avcodec_send_frame(codec_ctx_, hw_frame_);
        if (ret < 0) {
            throw std::runtime_error("发送帧到编码器失败");
        }

        // 接收编码后的包
        AVPacket* pkt = av_packet_alloc();
        while (ret >= 0) {
            ret = avcodec_receive_packet(codec_ctx_, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                av_packet_free(&pkt);
                throw std::runtime_error("编码错误");
            }

            // 处理编码后的数据（例如：发送到WebRTC）
            processEncodedPacket(pkt);
            av_packet_unref(pkt);
        }

        av_packet_free(&pkt);
        vaDestroySurfaces(va_display, &va_surface, 1);
    }

    void processEncodedPacket(AVPacket* pkt) {
        // 在此处将编码后的H.264数据发送到WebRTC栈
        // 例如：通过RTP传输或写入文件
        std::cout << "编码包大小: " << pkt->size << " bytes, PTS: " << pkt->pts << std::endl;
    }

    void createRenderPass() {
        // 创建Vulkan渲染通道（简化实现）
        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        // ... 配置渲染通道参数
        vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_);
    }

    void createFramebuffer() {
        // 创建帧缓冲区
        VkFramebufferCreateInfo fb_info = {};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass = render_pass_;
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = &vulkan_image_;
        fb_info.width = width_;
        fb_info.height = height_;
        fb_info.layers = 1;

        vkCreateFramebuffer(device_, &fb_info, nullptr, &framebuffer_);
    }

    void allocateCommandBuffer() {
        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = VK_NULL_HANDLE;  // 需要先创建命令池
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer_);
    }

    void run() {
        // 主循环：渲染+编码
        for (int frame_count = 0; frame_count < 300; ++frame_count) {  // 编码10秒（30fps）
            renderFrame();
            encodeFrame();

            // 模拟实时帧率（33ms/帧）
            usleep(33000);
        }
    }

    void cleanup() {
        // 清理Vulkan资源
        if (device_) {
            vkDeviceWaitIdle(device_);
            if (framebuffer_) vkDestroyFramebuffer(device_, framebuffer_, nullptr);
            if (render_pass_) vkDestroyRenderPass(device_, render_pass_, nullptr);
            if (vulkan_image_) vkDestroyImage(device_, vulkan_image_, nullptr);
            if (vulkan_memory_) vkFreeMemory(device_, vulkan_memory_, nullptr);
            vkDestroyDevice(device_, nullptr);
            vkDestroyInstance(instance_, nullptr);
        }

        // 清理FFmpeg资源
        if (hw_frame_) av_frame_free(&hw_frame_);
        if (codec_ctx_) avcodec_free_context(&codec_ctx_);
        if (hw_device_ctx_) av_buffer_unref(&hw_device_ctx_);

        // 关闭DMA-BUF文件描述符
        if (dmabuf_fd_ >= 0) close(dmabuf_fd_);
    }
};

int main() {
    try {
        VulkanVAAPIDemo demo(1920, 1080);  // 1080p分辨率
        demo.run();
        std::cout << "Demo执行成功" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}