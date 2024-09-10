#pragma once
#include <vulkan/vulkan.hpp>

#include "descriptors.hpp"
#include "geometryloader/geometry.hpp"
#include "pipeline.hpp"
#include "vknhandler.hpp"
#include <glm/fwd.hpp>


namespace rn {
class Raytracer {
public:
  Raytracer(std::shared_ptr<VulkanHandler> vlkn_, GeometryHandler &geom);
  ~Raytracer();
  vk::DeviceAddress getOutBufferAdress() { return rtPipeline.consts.out; };

private:
  std::shared_ptr<VulkanHandler> vlkn;
  void buildBlas(GeometryHandler &geom);
  void buildTlas();
  void buildDescriptorSet();
  void trace();
  void updatePushConstants(GeometryHandler &geom);
  void createOutputBuffer();

  vk::AccelerationStructureKHR blas;
  vk::AccelerationStructureKHR tlas;
  vk::Buffer blasBuffer;
  VmaAllocation blasAlloc;
  vk::Buffer tlasBuffer;
  VmaAllocation tlasAlloc;
  vk::Buffer instanceBuffer;
  VmaAllocation instanceAlloc;

  vk::Buffer outBuffer;
  VmaAllocation outAlloc;
  std::vector<glm::vec4> outData{1000};
  vk::DeviceAddress outAddress;

  vk::AccelerationStructureInstanceKHR instance;
  TraceDescriptors descriptor;
  vk::DescriptorSetLayout layout;
  vk::DescriptorPool pool;
  vk::DescriptorSet set;

  RaytracingPipeline rtPipeline;
};

} // namespace rn
