#pragma once

#include "descriptors.hpp"
#include "geometryloader/geometry.hpp"
#include "pipeline.hpp"
#include "vk_mem_alloc.h"
#include "vknhandler.hpp"

namespace rn {
class Raytracer {
public:
  Raytracer(std::shared_ptr<VulkanHandler> vlkn_, GeometryHandler &geom);
  ~Raytracer();

private:
  std::shared_ptr<VulkanHandler> vlkn;
  void buildBlas(GeometryHandler &geom);
  void buildTlas();
  void buildDescriptorSet();
  void trace();

  vk::AccelerationStructureKHR blas;
  vk::AccelerationStructureKHR tlas;
  vk::Buffer blasBuffer;
  VmaAllocation blasAlloc;
  vk::Buffer tlasBuffer;
  VmaAllocation tlasAlloc;
  vk::Buffer instanceBuffer;
  VmaAllocation instanceAlloc;

  vk::AccelerationStructureInstanceKHR instance;
  TraceDescriptors descriptor;
  vk::DescriptorSetLayout layout;
  vk::DescriptorPool pool;
  vk::DescriptorSet set;

  RaytracingPipeline rtPipeline;
};

} // namespace rn
