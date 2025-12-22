// src/main.cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" {
#include <libavutil/log.h>
}

#include "vulkan_export.cpp" // or include headers appropriately
#include "vaapi_import.cpp"
#include "ffmpeg_vaapi_encode.cpp"

int main(int argc, char** argv) {
    // demo params
    const int WIDTH = 1280, HEIGHT = 720;
    const char* RENDER_NODE = "/dev/dri/renderD128";
    const char* OUT_H264 = "out.h264";
    av_log_set_level(AV_LOG_INFO);

    // -------------------------
    // NOTE: 这里我们省略完整 Vulkan Instance / Device 创建代码（platform-dependent, 需要启用外部扩展）
    // 在实际工程中你需要：创建 VkInstance, 选择 physical device, 创建 VkDevice（开启 VK_KHR_external_memory_fd 等扩展）
    // 下面为了示例调用 create_exportable_image_and_export_fd 的接口，假定你已经有 instance/phys/device handles
    // -------------------------
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice phys = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    // TODO: 这里你必须填充 init Vulkan 的代码，启用扩展：
    //  - VK_KHR_external_memory_fd or VK_EXT_external_memory_dma_buf
    //  - VK_KHR_external_semaphore_fd (用于跨 API 同步，可选)
    fprintf(stderr,"NOTE: 请在 main 中补全 Vulkan 实例/设备创建（参见 README）\n");

    // For demonstration we will not actually call the Vulkan create if not implemented
    // Instead show the expected sequence:

    // 1) create_exportable_image_and_export_fd(...) -> get prime_fd
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    int prime_fd = -1;
    int rc = create_exportable_image_and_export_fd(instance, phys, device, VK_FORMAT_R8G8B8A8_UNORM, WIDTH, HEIGHT, &image, &memory, &prime_fd);
    if (rc != 0) {
        fprintf(stderr,"export image to dmabuf failed\n");
        return -1;
    }
    fprintf(stderr,"exported prime fd = %d\n", prime_fd);

    // 2) vaapi init
    int drm_fd = -1;
    VADisplay va_dpy = va_display_from_rendernode(RENDER_NODE, &drm_fd);
    if (!va_dpy) {
        fprintf(stderr,"va_display_from_rendernode failed\n");
        close(prime_fd);
        return -1;
    }
    fprintf(stderr,"va initialized on render node %s\n", RENDER_NODE);

    // For NV12 we'd need to give pitches/offsets; for the demo we just provide rough values
    int pitches[3] = { WIDTH * 1, WIDTH, 0 }; // placeholder; real values depend on format & driver
    int offsets[3] = { 0, WIDTH * HEIGHT, 0 };
    int num_planes = 2;
    unsigned fourcc = VA_FOURCC_NV12; // assuming NV12

    VASurfaceID va_surf = va_surface_from_dmabuf(va_dpy, prime_fd, WIDTH, HEIGHT, fourcc, pitches, offsets, num_planes);
    if (va_surf == VA_INVALID_ID) {
        fprintf(stderr,"va_surface_from_dmabuf failed\n");
        close(prime_fd);
        vaTerminate(va_dpy);
        close(drm_fd);
        return -1;
    }
    fprintf(stderr,"va surface created id=%u\n", (unsigned)va_surf);

    // 3) FFmpeg VAAPI device & encoder
    AVBufferRef* hwdev = create_vaapi_hwdevice(RENDER_NODE);
    if (!hwdev) {
        fprintf(stderr,"create_vaapi_hwdevice failed\n");
    } else {
        AVCodecContext* enc = create_vaapi_encoder(hwdev, WIDTH, HEIGHT, 2000000);
        if (!enc) {
            fprintf(stderr,"create_vaapi_encoder failed\n");
        } else {
            // IMPORTANT: 这里是关键：如何把 va_surf (libva 的 surface) 变成 AVFrame?
            // 这一步取决于 FFmpeg n8.0 的 API。常见做法：
            //  - 如果 FFmpeg 支持从 PRIME fd 直接 import 到 AVHWFramesContext，则先把 prime_fd import 至 hwframes pool，再 av_frame_get_buffer 得到 avframe。
            //  - 或者使用 av_hwframe_get_buffer + av_buffer_ref(enc_ctx->hw_frames_ctx) 之类的组合。
            //
            // 在这个 demo 中为了保守与兼容性，我示意地关闭这步并提醒你参考 ffmpeg/examples/vaapi_encode.c 去实现“从 prime fd -> AVFrame”的那段 glue 代码。
            //
            // 伪代码（示意）：
            // AVFrame *hwframe = create_avframe_from_va_surface(enc, va_surf);
            // encode_vaapi_frame(enc, hwframe, fopen(OUT_H264,"wb"));

            fprintf(stderr,"TODO: 把 va_surf 包装成 AVFrame 并调用 encode_vaapi_frame(enc, frame, file)\n");
        }
    }

    // Clean up
    // NOTE: close the prime fd if driver semantics require
    close(prime_fd);

    // vaTerminate & close drm fd
    vaTerminate(va_dpy);
    close(drm_fd);

    return 0;
}
