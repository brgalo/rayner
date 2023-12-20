#include "vma.hpp"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>
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

vk::Image VMA::creatDepthImage(VmaAllocation &alloc, VmaAllocationInfo &allocInfo, vk::ImageCreateInfo dImgInfo) {
  VmaAllocationCreateInfo info{.flags =
                                   VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                               .usage = VMA_MEMORY_USAGE_AUTO,
                               .priority = 1.0f};
  return createImage(alloc, allocInfo, dImgInfo, info);
}

vk::Image VMA::createImage(VmaAllocation &alloc, VmaAllocationInfo &allocInfo,
                           vk::ImageCreateInfo createInfo, VmaAllocationCreateInfo & allocCreateInfo) {
  VkImageCreateInfo temp(createInfo);
  VkImage imgTemp;
  auto res = vmaCreateImage(vma_, &temp, &allocCreateInfo, &imgTemp, &alloc,
                            &allocInfo);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create Image!");
  }
  return imgTemp;
}

vk::Buffer VMA::createBuffer(VmaAllocation &alloc, VmaAllocationInfo &allocInfo,
                             VkBufferCreateInfo &createInfo,
                             VmaAllocationCreateInfo &allocCreateInfo) {
  VkBufferCreateInfo bufInf = createInfo;
  VkBuffer temp;
  vmaCreateBuffer(vma_, &bufInf, &allocCreateInfo, &temp, &alloc, &allocInfo);
  return temp;
}

void VMA::destroyBuffer(VmaAllocation &alloc, vk::Buffer &buffer) {
  VkBuffer temp = buffer;
  vmaDestroyBuffer(vma_, temp, alloc);
}

} // namespace rn