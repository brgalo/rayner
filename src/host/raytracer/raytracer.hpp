#pragma once

#include "geometryloader/geometry.hpp"
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

  vk::AccelerationStructureKHR blas;
  vk::Buffer blasBuffer;
  VmaAllocation blasAlloc;
  vk::Buffer tlasBuffer;
  VmaAllocation tlasAlloc;
};

}