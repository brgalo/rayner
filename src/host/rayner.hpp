#pragma once

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
  Renderer renderer = Renderer(vlkn);
  Raytracer raytracer = Raytracer(vlkn, geom);
};
} // namespace rn