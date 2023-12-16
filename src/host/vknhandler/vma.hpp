#pragma once
#define VMA_VULKAN_VERSION 1002000
#include "vknhandler.hpp"

#ifndef VMA_H
#define VMA_H
#include "../../../libs/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#endif

namespace rn {
class VMA {
public:
  VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun);
  ~VMA() { vmaDestroyAllocator(vma_); }
  vk::Image creatDepthImage(VmaAllocation &alloc,
                       VmaAllocationInfo &allocInfo,
                       vk::ImageCreateInfo imgInfo);
private:
  void init();
  VmaAllocator vma_;
  const VkDevice dev;
};

} // namespace rn