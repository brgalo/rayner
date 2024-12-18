#include "window.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace rn {
std::vector<const char *> Window::getGlfwExtensions() {
  uint32_t count = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
  return std::vector<const char *>(glfwExtensions, glfwExtensions + count);
}

struct glfwContext {
  glfwContext() {
    glfwInit();
    glfwSetErrorCallback([](int error, const char *msg) {
      std::cerr << "glfw: (" << error << ") " << msg << std::endl;
    });
  }
  ~glfwContext() { glfwTerminate(); }
};

float Window::getAspectRatio() const {
  return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}

void Window::frameBufferResizedCallback(GLFWwindow *pWindow, int width,
                                        int height) {
  auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(pWindow));
  window->windowResized = true;
  window->extent.setWidth(width);
  window->extent.setHeight(height);
}

Window::Window(std::shared_ptr<VulkanHandler> vlkn)
    : physicalDevice(vlkn->getPhysDevice()) {
  instance = vlkn->getInstance();
  static auto glfwCtw = glfwContext();
  (void)glfwCtw;
  initWindowAndSwapchain();
}

Window::~Window() {
  instance->destroySurfaceKHR(surface);
  glfwDestroyWindow(window);
}

void Window::createSurface(vk::Instance instance) {
  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window,
                              nullptr, &_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  surface = vk::SurfaceKHR(_surface);
}

void Window::createWindow(uint32_t width_, uint32_t height_) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window = glfwCreateWindow(width_, height_, windowName.c_str(), nullptr, window);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
  extent.setWidth(width_);
  extent.setHeight(height_);
}

void Window::initWindowAndSwapchain() {
  createWindow(1000,500);
  createSurface(*instance);
}
} // namespace rn