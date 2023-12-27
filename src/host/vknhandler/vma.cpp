#include "vma.hpp"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"

namespace rn {

VMA::VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun) : dev(vkn_->getDevice()) , transferQ(vkn_->getTqueue()), transferPool(vkn_->getTpool()) {
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

void VMA::copyBuffer(vk::BufferCopy bufferCopy, vk::Buffer src, vk::Buffer dst) {
  vk::CommandBuffer buf =
      dev
          .allocateCommandBuffers(vk::CommandBufferAllocateInfo{
              transferPool, vk::CommandBufferLevel::ePrimary, 1})
          .front();
  buf.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  buf.copyBuffer(src, dst, bufferCopy);
  buf.end();

  vk::SubmitInfo info{};
  info.setCommandBuffers(buf);
  transferQ.submit(info);
  transferQ.waitIdle();
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

vk::Buffer VMA::uploadGeometry(const void *pData, vk::DeviceSize size, VmaAllocation &alloc) {
  vk::BufferCreateInfo bufferCreateInfo{
      {}, size, vk::BufferUsageFlagBits::eIndexBuffer};
  
  VmaAllocationCreateInfo createInfo{
      {}, VMA_MEMORY_USAGE_AUTO, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
  return uploadWithStaging(pData, size, alloc, vk::BufferUsageFlagBits::eVertexBuffer);
}

vk::Buffer VMA::uploadWithStaging(const void *pData, size_t size,
                                  VmaAllocation &alloc, vk::BufferUsageFlags usageFlags) {
  VmaAllocation stagingAlloc;
  VmaAllocationInfo stagingInfo;
  vk::Buffer stagingBuf = stagingBuffer(size, stagingAlloc, stagingInfo);

  memcpy(stagingInfo.pMappedData, pData, size);


  vk::BufferCreateInfo createInfo{
      {}, size, usageFlags | vk::BufferUsageFlagBits::eTransferDst};
  VmaAllocationInfo bufferInfo;
  VmaAllocationCreateInfo vmaInfo{{},VMA_MEMORY_USAGE_AUTO};

  vk::Buffer dest = createBuffer(alloc, bufferInfo, createInfo, vmaInfo);
  copyBuffer(vk::BufferCopy{0, 0, size}, stagingBuf, dest);
  destroyBuffer(stagingAlloc, stagingBuf);
  return dest;
}

vk::Buffer VMA::stagingBuffer(vk::DeviceSize size, VmaAllocation &alloc, VmaAllocationInfo &info) {
  vk::BufferCreateInfo bufferInfo{
      {}, size, vk::BufferUsageFlagBits::eTransferSrc};
  const VkBufferCreateInfo rawInfo = bufferInfo;
  VmaAllocationCreateInfo createInfo{
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
          VMA_ALLOCATION_CREATE_MAPPED_BIT,
      VMA_MEMORY_USAGE_AUTO};

  VkBuffer buf;
  vmaCreateBuffer(vma_, &rawInfo, &createInfo, &buf, &alloc, &info);
  return buf;
}
} // namespace rn