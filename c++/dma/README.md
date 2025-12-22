# vk_vaapi_zero_copy_demo

目标环境：Ubuntu 20.04，Vulkan 1.3.283，FFmpeg n8.0

## 依赖（示例）
请先安装常用依赖（有些包名可能需要调整）：
```bash
sudo apt update
sudo apt install build-essential cmake pkg-config git \
    libvulkan-dev libdrm-dev libva-dev libva-drm2 libva-x11-2 \
    libavcodec-dev libavutil-dev libavformat-dev
