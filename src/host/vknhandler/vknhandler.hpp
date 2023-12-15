#pragma once

#include <cstdint>
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
  uint32_t graphicsFamilyCount = 0;
  uint32_t presentFamily;
  uint32_t presentFamilyCount = 0;
  uint32_t computeFamily;
  uint32_t computeFamilyCount = 0;
  bool graphicsFamilyHasValue = false;
  bool graphicsHasPresentSupport = false;
  bool dedicatedComputeFamilyHasValue = false;
  bool isComplete() {
    return graphicsFamilyHasValue && graphicsHasPresentSupport &&
           dedicatedComputeFamilyHasValue;
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
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
  vk::Device device;
  QueueFamilyIndices queueFamilyIndices;
  Window window;

  vk::CommandPool gCommandPool;

  void createInstance();
  void createDebugCallback();
  void initWindowAndSwapchain();
  vk::PhysicalDevice pickPhysicalDevice();
  void createLogicalDevice(vk::PhysicalDevice physicalDevice);

  // helpers
  std::vector<char const *>
  getLayers(std::vector<char const *> const &layers,
            std::vector<vk::LayerProperties> const &layerProperties);
  std::vector<char const *> getExtensions(
      std::vector<char const *> const &extensions,
      std::vector<vk::ExtensionProperties> const &extensionProperties);
  bool isDeviceSuitable(vk::PhysicalDevice device);
  //  void hasGflwRequiredInstanceExtenstions() {
  //    auto extensions = vk::enumerateInstanceExtensionProperties();
  //  }

  const std::vector<const char *> deviceExtensionNames = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME};
  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
};
}