#pragma once


#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_MARGIN 16

#include "geometryloader/geometry.hpp"
#include "raytracer/raytracer.hpp"
#include "renderer.hpp"
#include "vknhandler.hpp"
#include <memory>


namespace rn {
class Rayner {
public:
  Rayner() {};
  void run();
private:
  std::shared_ptr<VulkanHandler> vlkn = std::make_shared<VulkanHandler>();
  GeometryHandler geom{vlkn->getVma()};
  Renderer renderer = Renderer(vlkn, geom);
  Raytracer raytracer = Raytracer(vlkn, geom);
};
} // namespace rn