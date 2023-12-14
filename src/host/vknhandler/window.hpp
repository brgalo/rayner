#pragma once

#include <vulkan/vulkan.hpp>

namespace rn {
class Window {
public:
  Window();
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

private:
  vk::SurfaceKHR surface;
};

}