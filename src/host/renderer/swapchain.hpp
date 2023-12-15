#pragma once

#include <vulkan/vulkan.hpp>

namespace rn {
class SwapChain {
public:
  SwapChain();
  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

private:
  vk::SurfaceFormatKHR choseSwapSurfaceFormat();
  
};
}