#include "raytracer.hpp"
#include "descriptors.hpp"
#include "vknhandler.hpp"
#include "vma.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
namespace rn {
Raytracer::Raytracer(std::shared_ptr<VulkanHandler> vlkn_,
                     GeometryHandler &geom)
    : vlkn(vlkn_), descriptor(vlkn_),
      rtPipeline(descriptor, vk::PipelineBindPoint::eRayTracingKHR,
                 vlkn) {
  buildBlas(geom);
  buildTlas();
  buildDescriptorSet();
  trace();
};


Raytracer::~Raytracer() {
  vlkn->getDevice().destroyDescriptorSetLayout(layout);
  vlkn->getDevice().destroyAccelerationStructureKHR(tlas);
  vlkn->getVma()->destroyBuffer(tlasAlloc, tlasBuffer);
  vlkn->getDevice().destroyAccelerationStructureKHR(blas);
  vlkn->getVma()->destroyBuffer(blasAlloc, blasBuffer);
  vlkn->getVma()->destroyBuffer(instanceAlloc, instanceBuffer);
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

  buildInfo.scratchData.deviceAddress =
      vlkn->getDevice().getBufferAddress(scratch);

  vk::CommandBuffer buffer = vlkn->beginSingleTimeCommands();
  buffer.buildAccelerationStructuresKHR(buildInfo, &rangeInfo);
  vlkn->endSingleTimeCommands(buffer);

  // destroy buffers
  vlkn->getVma()->destroyBuffer(scratchAlloc, scratch);
}

void Raytracer::buildTlas() {
  vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{blas};
  vk::DeviceAddress blasAddress =
      vlkn->getDevice().getAccelerationStructureAddressKHR(addressInfo);

  instance.transform.matrix[0][0] = 1.f;
  instance.transform.matrix[1][1] = 1.f;
  instance.transform.matrix[2][2] = 1.f;
  instance.instanceCustomIndex = 0;
  instance.mask = 0xFF;
  instance.instanceShaderBindingTableRecordOffset = 0;
  instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
  instance.accelerationStructureReference = blasAddress;

  VmaAllocationInfo instanceInfo;
  vk::BufferCreateInfo instanceBufferCreateInfo{
      {},
      sizeof(instance),
      vk::BufferUsageFlagBits::eTransferDst |
          vk::BufferUsageFlagBits::eShaderDeviceAddress};

  instanceBuffer = vlkn->getVma()->uploadInstanceB(instance, instanceAlloc);
  vk::DeviceAddress instanceBufferAddress = vlkn->getVma()->getDeviceAddress(instanceBuffer);

  vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{1, 0, 0, 0};
  vk::AccelerationStructureGeometryInstancesDataKHR instancesVK {
    VK_FALSE,instanceBufferAddress};

  vk::AccelerationStructureGeometryKHR geometry{vk::GeometryTypeKHR::eInstances,
                                                instancesVK};
  vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{
      vk::AccelerationStructureTypeKHR::eTopLevel,
      vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild,
      vk::BuildAccelerationStructureModeKHR::eBuild,
      VK_NULL_HANDLE,
      VK_NULL_HANDLE,
      1,
      &geometry};

  vk::AccelerationStructureBuildSizesInfoKHR sizeInfo =
      vlkn->getDevice().getAccelerationStructureBuildSizesKHR(
          vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, 1);

  vk::BufferCreateInfo tlasBufferCreateInfo{
      {},
      sizeInfo.accelerationStructureSize,
      vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer};

  VmaAllocationInfo tlasInfo{};
  VmaAllocationCreateInfo tlasAllocCreateInfo{
      {}, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT};
  tlasBuffer = vlkn->getVma()->createBuffer(
      tlasAlloc, tlasInfo, tlasBufferCreateInfo, tlasAllocCreateInfo);

  vk::AccelerationStructureCreateInfoKHR createInfo{
      {}, tlasBuffer, 0, sizeInfo.accelerationStructureSize, buildInfo.type};
  tlas = vlkn->getDevice().createAccelerationStructureKHR(createInfo);

  buildInfo.dstAccelerationStructure = tlas;

  // scratch buffer
  VmaAllocation scratchAlloc;
  VmaAllocationInfo scratchAllocInfo;
  vk::BufferCreateInfo scratchBufferCreateInfo{
      {},
      sizeInfo.buildScratchSize,
      vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress};
  VmaAllocationCreateInfo scratchInfo{
    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT,
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

  // destroy buffers
  vlkn->getVma()->destroyBuffer(scratchAlloc, scratch);
}

void Raytracer::buildDescriptorSet() { descriptor.writeSetup(tlas); }

void Raytracer::trace() {
  vk::CommandBuffer buffer = vlkn->beginSingleTimeCommands();
  rtPipeline.bind(buffer);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR,
                            rtPipeline.getLayout(), 0, 1,
                            &descriptor.getSets().front(), 0, nullptr);
  buffer.traceRaysKHR(rtPipeline.rgenRegion, rtPipeline.missRegion,
                      rtPipeline.hitRegion, {}, 1000, 1, 1);
  vlkn->endSingleTimeCommands(buffer);
  vlkn->getDevice().waitIdle();


}
// namespace rn
}