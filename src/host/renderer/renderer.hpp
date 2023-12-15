#pragma once

#include "swapchain.hpp"
#include "window.hpp"
#include <memory>
namespace rn {
class Renderer {
public:
  Renderer(std::shared_ptr<VulkanHandler> vlkn_) : vlkn(vlkn_) {};
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

private:
  std::shared_ptr<VulkanHandler> vlkn;
  Window window = Window{vlkn};
  SwapChain swapChain = SwapChain(vlkn, window);
};
}