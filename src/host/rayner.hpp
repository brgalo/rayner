#pragma once

#include "renderer.hpp"
#include "vknhandler.hpp"
#include "window.hpp"
#include <memory>
namespace rn {
class Rayner {
  std::shared_ptr<VulkanHandler> vlkn = std::make_shared<VulkanHandler>();

  Renderer renderer = Renderer(vlkn);
  };
}