#pragma once

#include <cstdint>
#include <memory>
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vknhandler.hpp"
#include <GLFW/glfw3.h>

namespace rn {
class Window {
public:
  Window() = delete;
  Window(std::shared_ptr<VulkanHandler> vlkn);
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  void createSurface(vk::Instance instance);
  void createWindow(uint32_t width, uint32_t height);
  const vk::SurfaceKHR getSurface() const {return surface;};

  static std::vector<const char *> getGlfwExtensions();
  const vk::Extent2D getExtent() const {
    int width = 0, height = 0;
    glfwGetWindowSize(window, &width, &height);
    return vk::Extent2D(width, height);
  };
  bool shouldClose() { return glfwWindowShouldClose(window); };
  bool wasResized() { return windowResized; }
  void poll() { glfwPollEvents(); };
  void resetResizedFlag() { windowResized = false; };
  GLFWwindow *get() { return window; };
  float getAspectRatio() const;

private:
  std::shared_ptr<vk::Instance> instance;
  vk::PhysicalDevice const &physicalDevice;
  void initWindowAndSwapchain();
  vk::SurfaceKHR surface;
  GLFWwindow *window = nullptr;
  vk::Extent2D extent;
  static void frameBufferResizedCallback(GLFWwindow *window, int width,
                                         int height);
  bool windowResized = false;

  std::string windowName = "RYNR v0.1";

  // helper
};

}