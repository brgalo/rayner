#pragma once

#include "vknhandler.hpp"
#include <vulkan/vulkan_core.h>
#ifndef VMA_H
#define VMA_H
#include "../../../libs/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#endif

namespace rn {
class VMA {
public:
  VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun);
  ~VMA() { vmaDestroyAllocator(vma); }

private:
  void init();
  VmaAllocator vma;
  const VkDevice dev;
};

} // namespace rn