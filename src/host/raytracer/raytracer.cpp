#include "raytracer.hpp"
#include "descriptors.hpp"
#include "pipeline.hpp"
#include "vknhandler.hpp"
#include "vma.hpp"
#include <cstdint>
#include <iostream>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rn {
Raytracer::Raytracer(std::shared_ptr<VulkanHandler> vlkn_,
                     GeometryHandler &geom)
    : vlkn(vlkn_), descriptor(vlkn_),
      rtPipeline(descriptor, vk::PipelineBindPoint::eRayTracingKHR,
                 vlkn) {
  buildBlas(geom);
  buildTlas();
  buildDescriptorSet();

  vk::FenceCreateInfo createInfo{vk::FenceCreateFlagBits::eSignaled};
  fence = vlkn->getDevice().createFence(createInfo);
  createOutputBuffer();
  updatePushConstants(geom);

  trace();
};


Raytracer::~Raytracer() {


  
  vlkn->getDevice().destroyDescriptorSetLayout(layout);
  vlkn->getDevice().destroyAccelerationStructureKHR(tlas);
  vlkn->getVma()->destroyBuffer(tlasAlloc, tlasBuffer);
  vlkn->getDevice().destroyAccelerationStructureKHR(blas);
  vlkn->getVma()->destroyBuffer(blasAlloc, blasBuffer);
  vlkn->getVma()->destroyBuffer(instanceAlloc, instanceBuffer);
  vlkn->getVma()->destroyBuffer(outAlloc, outBuffer);
  vlkn->getDevice().destroyFence(fence);
}

void Raytracer::buildBlas(GeometryHandler &geom) {
  vk::AccelerationStructureGeometryTrianglesDataKHR triangles{
    vk::Format::eR32G32B32A32Sfloat,
        vlkn->getVma()->getDeviceAddress(geom.getVert()),
        static_cast<uint32_t>(sizeof(glm::vec3)),
        static_cast<uint32_t>(geom.indices.size()),
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
  buffer.pushConstants(
      rtPipeline.getLayout(), vk::ShaderStageFlagBits::eRaygenKHR, 0,
      sizeof(RaytracingPipeline::RtConsts), &rtPipeline.consts);
  
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR,
                            rtPipeline.getLayout(), 0, 1,
                            &descriptor.getSets().front(), 0, nullptr);
  buffer.traceRaysKHR(rtPipeline.rgenRegion, rtPipeline.missRegion,
                      rtPipeline.hitRegion, {}, 100, 1, 1);

  vlkn->endSingleTimeCommands(buffer);
  
//buffer.end();
//  vk::SubmitInfo info{};
//  info.setCommandBufferCount(1);
//  info.pCommandBuffers = &buffer;
//
//  // reset fence
//  vlkn->getDevice().resetFences(fence);
//
//  vlkn->getGqueue().submit(info, fence);
//  if (vk::Result::eSuccess !=
//      vlkn->getDevice().waitForFences(fence, VK_TRUE, UINT64_MAX)) {
//    throw std::runtime_error("waited too long!");
//  };
//
  vlkn->getGqueue().waitIdle();
  vlkn->getDevice().waitIdle();

  const float *pData =
      reinterpret_cast<const float *>(outAllocInfo.pMappedData);

  for (size_t i = 0; i < outData.size(); ++i) {
    outData[i] = glm::vec4(pData[i * 4], pData[i * 4 + 1], pData[i * 4 + 2],
                           pData[i * 4 + 3]);
  }

}

void Raytracer::updatePushConstants(GeometryHandler &geom) {
  rtPipeline.consts.verts = vlkn->getVma()->getDeviceAddress(geom.getVert());
  rtPipeline.consts.idx = vlkn->getVma()->getDeviceAddress(geom.getIdx());
  rtPipeline.consts.out = vlkn->getVma()->getDeviceAddress(outBuffer);
}

void Raytracer::createOutputBuffer() {
  vk::DeviceSize size = sizeof(glm::vec3)*outData.size();
  vk::BufferCreateInfo outBufferCreateInfo{
      {},
      size,
      vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress};
  VmaAllocationCreateInfo outInfo{VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                                      VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                  VMA_MEMORY_USAGE_AUTO,
                                  VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT};

  outBuffer = vlkn->getVma()->createBuffer(outAlloc, outAllocInfo,
                                           outBufferCreateInfo, outInfo);

  rtPipeline.consts.out = vlkn->getVma()->getDeviceAddress(outBuffer);
}
  
// namespace rn
}