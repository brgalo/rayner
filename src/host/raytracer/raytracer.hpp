#pragma once
#include <memory>
#include <vector>
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
  RaytracingPipeline::RtConsts &getRtConstsPoints() {
    return rtPipelinePoints.consts;
  };
  RaytracingPipeline::RtConsts &getRtConstsRays() {
    return rtPipelineRays.consts;
  };
  void traceOri(std::shared_ptr<State> state);
  void traceRays(std::shared_ptr<State> state);

private:
  std::shared_ptr<VulkanHandler> vlkn;
  void buildBlas(GeometryHandler &geom);
  void buildTlas();
  void buildDescriptorSet();
  void updatePushConstantsPoints(GeometryHandler &geom);
  void updatePushConstantsRays(GeometryHandler &geom);
  void createOutputBuffer();
  void createOutputBufferRays(vk::DeviceSize hitBufferSize);

  vk::AccelerationStructureKHR blas;
  vk::AccelerationStructureKHR tlas;
  vk::Buffer blasBuffer;
  VmaAllocation blasAlloc;
  vk::Buffer tlasBuffer;
  VmaAllocation tlasAlloc;
  vk::Buffer instanceBuffer;
  VmaAllocation instanceAlloc;

  vk::Buffer outBuffer;
  vk::Buffer oriBuffer;
  vk::Buffer dirBuffer;
  vk::Buffer hitBuffer;
  VmaAllocation outAlloc;
  VmaAllocationInfo outAllocInfo;
  VmaAllocation oriAlloc;
  VmaAllocationInfo oriAllocInfo;
  VmaAllocation dirAlloc;
  VmaAllocationInfo dirAllocInfo;
  VmaAllocation hitAlloc;
  VmaAllocationInfo hitAllocInfo;
  std::vector<glm::vec4> outData{1000};

  vk::AccelerationStructureInstanceKHR instance;
  TraceDescriptors descriptor;
  vk::DescriptorSetLayout layout;
  vk::DescriptorPool pool;
  vk::DescriptorSet set;

  vk::Fence fence;

  RaytracingPipeline rtPipelinePoints;
  RaytracingPipeline rtPipelineRays;
  };

} // namespace rn
