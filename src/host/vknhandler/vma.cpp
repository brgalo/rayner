#include "vma.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_handles.hpp>
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"

namespace rn {

VMA::VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun) : dev(vkn_->getDevice()) , transferQ(vkn_->getTqueue()), transferPool(vkn_->getTpool()) {
  VmaAllocatorCreateInfo info {
    .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
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
  VkBuffer temp;
  vmaCreateBuffer(vma_, &createInfo, &allocCreateInfo, &temp, &alloc, &allocInfo);
  return temp;
}

void VMA::destroyBuffer(VmaAllocation &alloc, vk::Buffer &buffer) {
  VkBuffer temp = buffer;
  vmaDestroyBuffer(vma_, temp, alloc);
}

vk::Buffer VMA::uploadGeometry(const void *pData, vk::DeviceSize size, VmaAllocation &alloc) {
  return uploadWithStaging(pData, size, alloc,
                           vk::BufferUsageFlagBits::eVertexBuffer,
                           {{}, VMA_MEMORY_USAGE_GPU_ONLY});
}

vk::Buffer VMA::uploadVertices(const std::vector<glm::vec3> &verts,
                               VmaAllocation &alloc) {
  return uploadWithStaging(
      verts.data(), sizeof(verts[0]) * verts.size(), alloc,
      vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
      {{}, VMA_MEMORY_USAGE_GPU_ONLY});
}

vk::Buffer VMA::uploadIndices(const std::vector<uint32_t> &idx, VmaAllocation &alloc) {
  return uploadWithStaging(
      idx.data(), sizeof(idx[0]) * idx.size(), alloc,
      vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
      {{}, VMA_MEMORY_USAGE_GPU_ONLY});
}

vk::Buffer VMA::uploadInstanceB(const vk::AccelerationStructureInstanceKHR &instance, VmaAllocation &alloc) {
  VmaAllocationCreateInfo instanceAllocCreateInfo{
      VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
      VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT};

  return uploadWithStaging(
      &instance, sizeof(instance), alloc,
      vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
      instanceAllocCreateInfo);
}

void VMA::updateDescriptor(const void *pData, vk::DeviceSize size,
                           VmaAllocationInfo &info) {
  memcpy(info.pMappedData, pData, size);
}

vk::DeviceAddress VMA::getDeviceAddress(vk::Buffer buffer) {
  return dev.getBufferAddress(buffer);
}

vk::Buffer VMA::uploadWithStaging(const void *pData, size_t size,
                                  VmaAllocation &alloc, vk::BufferUsageFlags usageFlags, VmaAllocationCreateInfo allocCreateInfo) {
  VmaAllocation stagingAlloc;
  VmaAllocationInfo stagingInfo;
  vk::Buffer stagingBuf = stagingBuffer(size, stagingAlloc, stagingInfo);

  memcpy(stagingInfo.pMappedData, pData, size);

  std::vector<float> test(size);
  memcpy(test.data(), stagingInfo.pMappedData, size);

  vk::BufferCreateInfo createInfo{
      {}, size, usageFlags | vk::BufferUsageFlagBits::eTransferDst};
  VmaAllocationInfo bufferInfo;
//  VmaAllocationCreateInfo vmaInfo{{},VMA_MEMORY_USAGE_AUTO};

  vk::Buffer dest = createBuffer(alloc, bufferInfo, createInfo, allocCreateInfo);
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