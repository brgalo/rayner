#pragma once
#include "vknhandler.hpp"
#include <cstdint>
#include <vector>


#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "glm/glm.hpp"

#ifndef VMA_H
#define VMA_H
#include "vk_mem_alloc.h"
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
  void destroyBuffer(VmaAllocation &alloc, vk::Buffer &buffer) const;
  //vk::Buffer a;

  vk::Buffer uploadGeometry(const void *pData, vk::DeviceSize size,
                            VmaAllocation &alloc);
  vk::Buffer uploadVertices(const std::vector<glm::vec3> &verts,
                            VmaAllocation &alloc);
  vk::Buffer uploadIndices(const std::vector<uint32_t> &idx,
                           VmaAllocation &alloc);
  vk::Buffer uploadInstanceB(const vk::AccelerationStructureInstanceKHR &instance,
                             VmaAllocation &alloc);
  void updateDescriptor(const void *pData, vk::DeviceSize size,
                        VmaAllocationInfo &info);

  vk::DeviceAddress getDeviceAddress(vk::Buffer buffer);

  VmaAllocator& vma() {return vma_;};
private:
  void init();
  VmaAllocator vma_;
  const vk::Device dev;
  const vk::Queue transferQ;
  const vk::CommandPool transferPool;

  vk::Buffer uploadWithStaging(const void *pData, size_t site,
                               VmaAllocation &alloc,
                               vk::BufferUsageFlags usageFlags,
                               VmaAllocationCreateInfo allocCreateInfo);

  void copyBuffer(vk::BufferCopy bufferCopy, vk::Buffer src, vk::Buffer dst);

private:
  // function for internal use only
  vk::Image createImage(VmaAllocation &alloc, VmaAllocationInfo &allocInfo,
                        vk::ImageCreateInfo createInfo,
                        VmaAllocationCreateInfo &allocCreateInfo);

  vk::Buffer stagingBuffer(vk::DeviceSize size, VmaAllocation &alloc,
                           VmaAllocationInfo &info);
};

} // namespace rn