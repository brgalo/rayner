#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <iterator>
#include "vknhandler.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>


#ifndef DYN_VULKAN
#define DYN_VULKAN

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

namespace rn {


static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
              void *pUserData) {
  std::cerr << vk::to_string(
      static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity))
            << ": "
            << vk::to_string(
                   static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType))
            << ":\n";
  std::cerr << std::string("\t") << "messageIdName   = <"
            << pCallbackData->pMessageIdName << ">\n";
  std::cerr << std::string("\t") << "messageIdNumber = <"
            << pCallbackData->messageIdNumber << ">\n";
  std::cerr << std::string("\t") << "message            = <"
            << pCallbackData->pMessage << ">\n";
  

  return VK_FALSE;
}

VulkanHandler::VulkanHandler() {
  createInstance();
  createDebugCallback();
  initWindowAndSwapchain();
  createLogicalDevice(pickPhysicalDevice());
  createQueues();
  createCommandPools();
}

VulkanHandler::~VulkanHandler() {
  device.destroyCommandPool(gPool);
  device.destroyCommandPool(cPool);
  device.destroyCommandPool(tPool);

  device.destroy();

  instance.destroySurfaceKHR(window.getSurface());
  instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
  instance.destroy();
}

void VulkanHandler::createInstance() {
  // calls the dynamically loaded functions
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
  // first init the instance
  vk::ApplicationInfo appInfo(applicationName.c_str(), 1, engineName.c_str(), 1,
                              vk::ApiVersion13);

  // gather all Layers & extensions
  auto instanceLayerProps = vk::enumerateInstanceLayerProperties();
  std::vector<char const *> instanceLayerNames({"VK_LAYER_KHRONOS_validation"});
  std::vector<char const *> enabledLayers =
      getLayers(instanceLayerNames, instanceLayerProps);

  auto instanceExtensionProps = vk::enumerateInstanceExtensionProperties();
  std::vector<char const *> instanceExtensionNames(
      {VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
  std::vector<char const *> enabledExtensions = getExtensions(instanceExtensionNames, instanceExtensionProps);
  vk::InstanceCreateInfo instCreateInfo({}, &appInfo, enabledLayers, enabledExtensions);

  // store in member variable
  instance = vk::createInstance(instCreateInfo);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}

void VulkanHandler::createDebugCallback() {  
  debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
      vk::DebugUtilsMessengerCreateInfoEXT(
          {},
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
              vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation, &debugCallback));
}

void VulkanHandler::initWindowAndSwapchain() {
  window.createWindow(1000,500);
  window.createSurface(instance);
}

vk::PhysicalDevice VulkanHandler::pickPhysicalDevice() {
  auto devices = instance.enumeratePhysicalDevices();

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
    //  std::cout << "physical device: " << device.getProperties().deviceName << std::endl;
      return device;
    }
  }
  throw std::runtime_error("No suitable device present!");
}

std::vector<char const *> VulkanHandler::getLayers(
    std::vector<char const *> const &layers,
    std::vector<vk::LayerProperties> const &layerProperties) {
  std::vector<char const *> enabledLayers;
  enabledLayers.reserve(layers.size());
  for (auto const &layer : layers) {
    assert(std::any_of(layerProperties.begin(), layerProperties.end(),
                       [layer](vk::LayerProperties const &lp) {
                         return strcmp(layer,lp.layerName)==0;
                       }));
    enabledLayers.push_back(layer);
  }

  if (std::none_of(layers.begin(), layers.end(),
                   [](std::string const &layer) {
                     return layer == "VK_LAYER_KHRONS_validation";
                   }) &&
      std::any_of(layerProperties.begin(), layerProperties.end(),
                  [](vk::LayerProperties const &lp) {
                    return (strcmp("VK_LAYER_KHRONOS_validation",
                                    lp.layerName) == 0);
                  })) {
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
  }
  return enabledLayers;
}

std::vector<char const *> VulkanHandler::getExtensions(
    std::vector<char const *> const &extensions,
    std::vector<vk::ExtensionProperties> const &extensionProperties) {
  std::vector<char const *> enabledExtensions;
  enabledExtensions.reserve(extensions.size());
  for (auto const &ext : extensions) {
    assert(std::any_of(extensionProperties.begin(), extensionProperties.end(),
                       [ext](vk::ExtensionProperties const &ep) {
                         return strcmp(ext,ep.extensionName) == 0;
                       }));
    enabledExtensions.push_back(ext);
  }
  if(std::none_of(extensions.begin(),extensions.end(),[](std::string const & extension) {return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME;}) &&
           std::any_of(extensionProperties.begin(), extensionProperties.end(),
                       [](vk::ExtensionProperties const &ep) {
                         return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                        ep.extensionName) == 0);
                       })) {
      enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  // lead gltf Extension, add here to support other window framework
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  enabledExtensions.insert(std::end(enabledExtensions),glfwExtensions,glfwExtensions+glfwExtensionCount);
  return enabledExtensions;
}

bool VulkanHandler::isDeviceSuitable(vk::PhysicalDevice device) {

  // check if graphics and present queues are present
  QueueFamilyIndices indices;
  auto queueFamilies = device.getQueueFamilyProperties();
  uint32_t idx = 0;
  for (const auto qf : queueFamilies) {
      if (qf.queueCount > 0 && qf.queueFlags & vk::QueueFlagBits::eGraphics &&
          !indices.graphicsFamilyHasValue) {
      indices.graphicsFamily = idx;
      indices.computeFamilyCount = qf.queueCount;
      indices.graphicsFamilyHasValue = true;
      indices.graphicsHasPresentSupport = device.getSurfaceSupportKHR(
          static_cast<uint32_t>(idx), window.getSurface());
      }
      // check for dedicated compute family.
      if (qf.queueCount > 0 && qf.queueFlags & vk::QueueFlagBits::eCompute &&
          !(qf.queueFlags & vk::QueueFlagBits::eGraphics) &&
          !indices.dedicatedComputeFamilyHasValue) {
      indices.computeFamily = idx;
      indices.dedicatedComputeFamilyHasValue = true;
      }
      // check for dedicated transfer family.
      if (qf.queueCount > 0 && qf.queueFlags & vk::QueueFlagBits::eTransfer &&
          !(qf.queueFlags & vk::QueueFlagBits::eGraphics) &&
          !(qf.queueFlags & vk::QueueFlagBits::eCompute)) {
      indices.transferFamily = idx;
      indices.transferFamilyHasValue = true;
      indices.transferFamilyCount = qf.queueCount;
      }
      if (indices.isComplete()) {
      break;
      }
      idx++;
  }
  // set graphics queue family as compute queue family, if no dedicated compute
  // familily was found. This is mostly the case for standard GPU hardware
  if (!indices.dedicatedComputeFamilyHasValue) {
      indices.computeFamily = indices.graphicsFamily;
      indices.dedicatedComputeFamilyHasValue = true;
  }

  bool extensionSupported = false;
  std::vector<vk::ExtensionProperties> devExtensions = device.enumerateDeviceExtensionProperties();
  std::set<std::string> reqExtensions(deviceExtensionNames.begin(),
                                      deviceExtensionNames.end());
  for (const auto &dE : devExtensions) {
      reqExtensions.erase(dE.extensionName.data());
    //  std::cout << dE.extensionName << std::endl;
  }
  if (reqExtensions.empty()) {
      queueFamilyIndices = indices;
      extensionSupported = true;
  }

  // determine capabilities of swapchain
  SwapChainSupportDetails details;

  return extensionSupported && indices.isComplete();
}

void VulkanHandler::createLogicalDevice(vk::PhysicalDevice physicalDevice) {
  // is ignored by most drivers
  float queuePrio = 0.f;
  vk::DeviceQueueCreateInfo graphicsInfo({}, queueFamilyIndices.graphicsFamily,
                                         1, &queuePrio);
  vk::DeviceQueueCreateInfo computeInfo({}, queueFamilyIndices.computeFamily,
                                        1, &queuePrio);
  vk::DeviceQueueCreateInfo transferInfo({}, queueFamilyIndices.transferFamily, 1,
                                        &queuePrio);
  const std::array<const vk::DeviceQueueCreateInfo, 3> queueInfos{graphicsInfo,computeInfo,transferInfo};
{}
  vk::DeviceCreateInfo createInfo({}, queueInfos, {}, deviceExtensionNames);
  device = physicalDevice.createDevice(createInfo);

  // load functions dynamically
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
}

void VulkanHandler::createQueues() {
  gQueue = device.getQueue(queueFamilyIndices.graphicsFamily, 0);
  cQueue = device.getQueue(queueFamilyIndices.computeFamily, 0);
  tQueue = device.getQueue(queueFamilyIndices.transferFamily, 0);
}

void VulkanHandler::createCommandPools() {
  gPool = device.createCommandPool(
      vk::CommandPoolCreateInfo({}, queueFamilyIndices.graphicsFamily));
  cPool = device.createCommandPool(
      vk::CommandPoolCreateInfo({}, queueFamilyIndices.computeFamily));
  tPool = device.createCommandPool(
      vk::CommandPoolCreateInfo({}, queueFamilyIndices.transferFamily));
}
}