#include "raytracer.hpp"
#include "vma.hpp"
#include "vknhandler.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rn {
Raytracer::Raytracer(std::shared_ptr<VulkanHandler> vlkn_, GeometryHandler &geom) : vlkn(vlkn_) {
  buildBlas(geom);
}

Raytracer::~Raytracer() {
  
}

void Raytracer::buildBlas(GeometryHandler &geom) {
  vk::AccelerationStructureGeometryTrianglesDataKHR triangles{
      vk::Format::eR32G32B32Sfloat,
      vlkn->getVma()->getDeviceAddress(geom.getVert()),
      static_cast<uint32_t>(sizeof(glm::vec3)),
      2,
      vk::IndexType::eUint32,
      vlkn->getVma()->getDeviceAddress(geom.getIdx()),
      {}};

  vk::AccelerationStructureGeometryKHR geometryInfo{
      vk::GeometryTypeKHR::eTriangles, triangles,
      vk::GeometryFlagBitsKHR::eOpaque};

  vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{
      static_cast<uint32_t>(geom.indices.size() / 3), 0, 0, 0};

  vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{
      vk::AccelerationStructureTypeKHR::eBottomLevel,
      {},
      vk::BuildAccelerationStructureModeKHR::eBuild,
      VK_NULL_HANDLE,
      VK_NULL_HANDLE,
      1,
      &geometryInfo};

  vk::AccelerationStructureBuildSizesInfoKHR sizeInfo =
      vlkn->getDevice().getAccelerationStructureBuildSizesKHR(
          vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, 2);

  vk::BufferCreateInfo blasBufferCreateInfo{
      {},
      sizeInfo.accelerationStructureSize,
      vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR};
  VmaAllocationInfo blasInfo{};
  VmaAllocationCreateInfo blasAllocCreateInfo{
      {}, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, {}};

  blasBuffer = vlkn->getVma()->createBuffer(
      blasAlloc, blasInfo, blasBufferCreateInfo, blasAllocCreateInfo);

  vk::AccelerationStructureCreateInfoKHR createInfo{
      {},
      blasBuffer,
      0,
      sizeInfo.accelerationStructureSize,
      buildInfo.type};

  blas = vlkn->getDevice().createAccelerationStructureKHR(createInfo);
  buildInfo.dstAccelerationStructure = blas;

  // scratch buffer
  VmaAllocation scratchAlloc;
  VmaAllocationInfo scratchAllocInfo;
  vk::BufferCreateInfo scratchBufferCreateInfo{
      {},
      sizeInfo.buildScratchSize,
      vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress};
  VmaAllocationCreateInfo scratchInfo{
      VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY,
      VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
      {}};
  vk::Buffer scratch = vlkn->getVma()->createBuffer(
      scratchAlloc, scratchAllocInfo, scratchBufferCreateInfo, scratchInfo);

  vk::MemoryAllocateInfo allocInfo{sizeInfo.buildScratchSize};
  buildInfo.scratchData.deviceAddress =
      vlkn->getDevice().getBufferAddress(scratch);

  vk::CommandBuffer buffer = vlkn->beginSingleTimeCommands();
  buffer.buildAccelerationStructuresKHR(buildInfo, &rangeInfo);
  vlkn->endSingleTimeCommands(buffer);
}
}