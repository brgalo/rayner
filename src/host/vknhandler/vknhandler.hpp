#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

namespace rn {
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

  void createInstance();
  void createDebugCallback();
  void initPhysicalDevice();


  std::vector<char const *>
  getLayers(std::vector<std::string> const &layers,
            std::vector<vk::LayerProperties> const &layerProperties);
  std::vector<char const *> getExtensions(
      std::vector<std::string> const &extensions,
      std::vector<vk::ExtensionProperties> const &extensionProperties);

  PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
};
}