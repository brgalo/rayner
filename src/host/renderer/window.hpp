#pragma once

#include <cstdint>
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "swapchain.hpp"

namespace rn {
class Window {
public:
  Window();
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  void createSurface(vk::Instance instance);
  void createWindow(uint32_t width, uint32_t height);
  const vk::SurfaceKHR getSurface() const {return surface;};

  static std::vector<const char*> getGlfwExtensions();
private:
  vk::SurfaceKHR surface;
  GLFWwindow *window = nullptr;
  static void frameBufferResizedCallback(GLFWwindow *window, int width,
                                         int height);

  uint32_t width ;
  uint32_t height;
  bool windowResized = false;

  std::string windowName = "RYNR v0.1";

  // helper
};

}