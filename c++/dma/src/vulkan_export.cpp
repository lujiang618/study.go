// src/vulkan_export.cpp
#include <vulkan/vulkan.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

// 简化错误宏
#define CHECK_VK(x,msg) if ((x) != VK_SUCCESS) { fprintf(stderr,"VKERR: %s\n",msg); return -1; }

static uint32_t find_memory_type(VkPhysicalDevice phys, uint32_t typeFilter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) return i;
    }
    return UINT32_MAX;
}

int create_exportable_image_and_export_fd(VkInstance instance, VkPhysicalDevice phys, VkDevice device,
                                          VkFormat format, uint32_t width, uint32_t height,
                                          VkImage* outImage, VkDeviceMemory* outMemory, int* out_fd) {
    // create image with VkExternalMemoryImageCreateInfo in pNext
    VkImageCreateInfo imgInfo = {};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = format;
    imgInfo.extent = { width, height, 1 };
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkExternalMemoryImageCreateInfo extImg = {};
    extImg.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    extImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT; // dma-buf for Linux
    imgInfo.pNext = &extImg;

    VkResult r = vkCreateImage(device, &imgInfo, NULL, outImage);
    CHECK_VK(r, "vkCreateImage failed");

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, *outImage, &memReq);

    uint32_t memIndex = find_memory_type(phys, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (memIndex == UINT32_MAX) {
        fprintf(stderr,"no memtype\n"); return -1;
    }

    VkExportMemoryAllocateInfo exportAlloc = {};
    exportAlloc.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    exportAlloc.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = memIndex;
    allocInfo.pNext = &exportAlloc;

    r = vkAllocateMemory(device, &allocInfo, NULL, outMemory);
    CHECK_VK(r, "vkAllocateMemory failed");

    r = vkBindImageMemory(device, *outImage, *outMemory, 0);
    CHECK_VK(r, "vkBindImageMemory failed");

    // NOTE: 在真实情况你应该提交渲染到这个 image 的命令并确保 GPU 完成（外部同步 or simple device wait)
    // 这里为了示例简单，我们 assume 已经渲染完成（或者调用者会确保）

    // 导出 fd
    auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
    if (!vkGetMemoryFdKHR) {
        fprintf(stderr,"no vkGetMemoryFdKHR\n"); return -1;
    }

    VkMemoryGetFdInfoKHR getFd = {};
    getFd.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    getFd.memory = *outMemory;
    getFd.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    int fd = -1;
    r = vkGetMemoryFdKHR(device, &getFd, &fd);
    if (r != VK_SUCCESS || fd < 0) {
        fprintf(stderr,"vkGetMemoryFdKHR failed\n"); return -1;
    }

    *out_fd = fd;
    return 0;
}
