#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#ifndef VMA_H
#define VMA_H
#include "../../../libs/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#endif

namespace rn {

class VMA;

// helper structs
struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t graphicsFamilyCount = 0;
  uint32_t transferFamily;
  uint32_t transferFamilyCount = 0;
  uint32_t computeFamily;
  uint32_t computeFamilyCount = 0;
  bool graphicsFamilyHasValue = false;
  bool graphicsHasPresentSupport = false;
  bool transferFamilyHasValue = false;
  bool dedicatedComputeFamilyHasValue = false;
  bool isComplete() {
    return graphicsFamilyHasValue && graphicsHasPresentSupport &&
           dedicatedComputeFamilyHasValue && transferFamilyHasValue;
  }
};

// class that interacts with vulkan directly
class VulkanHandler {
public:
  VulkanHandler();
  ~VulkanHandler();

  VulkanHandler(const VulkanHandler &) = delete;
  VulkanHandler &operator=(const VulkanHandler &) = delete;

  const std::shared_ptr<vk::Instance> getInstance() const { return instance; };
  const vk::PhysicalDevice &getPhysDevice() const { return physicalDevice; };
  const vk::Device &getDevice() const { return device; };
  std::shared_ptr<VMA> getVma() { return vma; };
  const vk::CommandPool &getGpool() const { return gPool; };


  vk::ShaderModule createShaderModule(std::vector<char> &code);
  void destroyShaderModule(vk::ShaderModule &module);


private:
  std::shared_ptr<VMA> vma = nullptr;
  const std::string applicationName = "rayner";
  const std::string engineName = "nopnts";

  std::shared_ptr<vk::Instance> instance;
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  QueueFamilyIndices queueFamilyIndices;
  vk::Queue gQueue;
  vk::Queue cQueue;
  vk::Queue tQueue;
  vk::CommandPool gPool;
  vk::CommandPool cPool;
  vk::CommandPool tPool;

  void createInstance();
  void createDebugCallback();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createVMA();
  void createQueues();
  void createCommandPools();

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
} // namespace rn