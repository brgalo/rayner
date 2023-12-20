#pragma once
#include "vknhandler.hpp"
#include <vector>
#include <vulkan/vulkan_structs.hpp>

#ifndef VMA_H
#define VMA_H
#include "../../../libs/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#endif

namespace rn {
class VMA {
public:
  VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun);
  ~VMA() { vmaDestroyAllocator(vma_); }
  vk::Image creatDepthImage(VmaAllocation &alloc, VmaAllocationInfo &allocInfo,
                            vk::ImageCreateInfo dImgInfo);
  void destroyImage(VkImage img, VmaAllocation alloc) {
    vmaDestroyImage(vma_, img, alloc);
  };

  vk::Buffer createBuffer(VmaAllocation &alloc, VmaAllocationInfo &allocInfo,
                          VkBufferCreateInfo &createInfo,
                          VmaAllocationCreateInfo &allocCreateInfo);
  void destroyBuffer(VmaAllocation &alloc, vk::Buffer &buffer);
  VmaAllocator& vma() {return vma_;};
  private:
  void init();
  VmaAllocator vma_;
  const VkDevice dev;

private:
  // function for internal use only
  vk::Image createImage(VmaAllocation &alloc, VmaAllocationInfo &allocInfo,
                        vk::ImageCreateInfo createInfo,
                        VmaAllocationCreateInfo &allocCreateInfo);
};

} // namespace rn