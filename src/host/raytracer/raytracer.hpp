#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

#include "descriptors.hpp"
#include "geometryloader/geometry.hpp"
#include "pipeline.hpp"
#include "vknhandler.hpp"
#include <glm/fwd.hpp>
#include <vulkan/vulkan_handles.hpp>
#include "state.hpp"


namespace rn {
class Raytracer {
public:
  Raytracer(std::shared_ptr<VulkanHandler> vlkn_, GeometryHandler &geom);
  ~Raytracer();
  RaytracingPipeline::RtConsts &getRtConsts() { return rtPipeline.consts; };
  void trace(std::shared_ptr<State> state);

private:
  std::shared_ptr<VulkanHandler> vlkn;
  void buildBlas(GeometryHandler &geom);
  void buildTlas();
  void buildDescriptorSet();
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
  VmaAllocationInfo outAllocInfo;
  std::vector<glm::vec4> outData{1000};

  vk::AccelerationStructureInstanceKHR instance;
  TraceDescriptors descriptor;
  vk::DescriptorSetLayout layout;
  vk::DescriptorPool pool;
  vk::DescriptorSet set;

  vk::Fence fence;

  RaytracingPipeline rtPipeline;
};

} // namespace rn
