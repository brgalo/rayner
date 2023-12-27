#pragma once

#include "geometryloader/geometry.hpp"
#include "renderer.hpp"
#include "vknhandler.hpp"
#include <memory>
namespace rn {
class Rayner {
public:
  Rayner() { renderer.render(geom.getVert()); }
private:
  std::shared_ptr<VulkanHandler> vlkn = std::make_shared<VulkanHandler>();
  GeometryHandler geom{vlkn->getVma()};
  Renderer renderer = Renderer(vlkn);
};
} // namespace rn