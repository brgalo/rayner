#pragma once

#include <cstdint>
#include <memory>
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "../vknhandler/vknhandler.hpp"

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
  const vk::Extent2D& getExtent() const {return extent;};

private:
  std::shared_ptr<vk::Instance> instance;
  vk::PhysicalDevice &physicalDevice;
  void initWindowAndSwapchain();
  vk::SurfaceKHR surface;
  GLFWwindow *window = nullptr;
  vk::Extent2D extent;
  static void frameBufferResizedCallback(GLFWwindow *window, int width,
                                         int height);

  uint32_t width ;
  uint32_t height;
  bool windowResized = false;

  std::string windowName = "RYNR v0.1";

  // helper
};

}