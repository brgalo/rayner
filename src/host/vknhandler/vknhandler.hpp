#pragma once

#include <vector>
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>

#include "window.hpp"

namespace rn {

// helper structs
struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

// class that interacts with vulkan directly
class VulkanHandler {
public:
  VulkanHandler();
  ~VulkanHandler();

  VulkanHandler(const VulkanHandler &) = delete;
  VulkanHandler &operator=(const VulkanHandler &) = delete;

private:
  const std::string applicationName = "rayner";
  const std::string engineName = "nopnts";

  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
  vk::PhysicalDevice physicalDevice;
  vk::SurfaceKHR surface;
  QueueFamilyIndices queueFamilyIndices;
  Window window;

  void createInstance();
  void createDebugCallback();
  void initWindowAndSwapchain();
  void initPhysicalDevice();

  // helpers
  std::vector<char const *>
  getLayers(std::vector<char const *> const &layers,
            std::vector<vk::LayerProperties> const &layerProperties);
  std::vector<char const *> getExtensions(
      std::vector<char const *> const &extensions,
      std::vector<vk::ExtensionProperties> const &extensionProperties);
  bool isDeviceSuitable(vk::PhysicalDevice device);
  void hasGflwRequiredInstanceExtenstions() {
    auto extensions = vk::enumerateInstanceExtensionProperties();
  }
};
}