#include "vma.hpp"
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"

namespace rn {
VMA::VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun) : dev(vkn_->getDevice()) {
  VmaAllocatorCreateInfo info{
    .physicalDevice = vkn_->getPhysDevice(), .device = dev,
    .pVulkanFunctions = &fun, .instance = *vkn_->getInstance(),
    .vulkanApiVersion = VK_API_VERSION_1_2};
  vmaCreateAllocator(&info, &vma_);
};

vk::Image VMA::creatDepthImage(VmaAllocation &alloc, VmaAllocationInfo &allocInfo, vk::ImageCreateInfo imgInfo) {
  VmaAllocationCreateInfo info {
    .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
    .usage = VMA_MEMORY_USAGE_AUTO,
    .priority = 1.0f};
  VkImageCreateInfo temp(imgInfo);
  VkImage imgTemp;
  auto res = vmaCreateImage(vma_, &temp, &info, &imgTemp, &alloc, &allocInfo);
  return imgTemp;
}

} // namespace rn