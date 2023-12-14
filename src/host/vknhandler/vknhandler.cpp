#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "vknhandler.hpp"
#include <algorithm>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vulkan/vulkan_core.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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
  initPhysicalDevice();
}

VulkanHandler::~VulkanHandler() {
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
  std::vector<vk::LayerProperties> instanceLayerProps = vk::enumerateInstanceLayerProperties();
  std::vector<char const *> instanceLayerNames({"VK_LAYER_KHRONOS_validation"});
  std::vector<char const *> enabledLayers =
      getLayers(instanceLayerNames, instanceLayerProps);

  std::vector<char const *> instanceExtensionNames(
      {VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
  

  vk::InstanceCreateInfo instCreateInfo({}, &appInfo, instanceLayerNames, instanceExtensionNames);

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

void VulkanHandler::initPhysicalDevice() {
  auto devices = instance.enumeratePhysicalDevices();

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice = device;
      break;
    }
  }
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
    std::vector<std::string> const &extensions,
    std::vector<vk::ExtensionProperties> const &extensionProperties) {
  std::vector<char const *> enabledExtenstions;
  enabledExtenstions.reserve(extensions.size());
  for (auto const &ext : extensions) {
    assert(std::any_of(extensionProperties.begin(), extensionProperties.end(),
                       [ext](vk::ExtensionProperties const &ep) {
                         return ext == ep.extensionName;
                       }));
    enabledExtenstions.push_back(ext.data());
  }
  if(std::none_of(extensions.begin(),extensions.end(),[](std::string const & extension) {return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME;}) &&
           std::any_of(extensionProperties.begin(), extensionProperties.end(),
                       [](vk::ExtensionProperties const &ep) {
                         return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                        ep.extensionName) == 0);
                       })) {
      enabledExtenstions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return enabledExtenstions;
}

bool VulkanHandler::isDeviceSuitable(vk::PhysicalDevice device) {

  // check if graphics and present queues are present
  QueueFamilyIndices indices;
  auto queueFamilies = device.getQueueFamilyProperties();
  uint32_t idx = 0;
  for (const auto qf : queueFamilies) {
      if (qf.queueCount > 0 && qf.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = idx;
      indices.graphicsFamilyHasValue = true;
      }
      vk::Bool32 presentSupport = device.getSurfaceSupportKHR(static_cast<uint32_t>(idx), surface);
      if (qf.queueCount > 0 && presentSupport) {
      indices.presentFamily = idx;
      indices.presentFamilyHasValue = true;
      }
      if (indices.isComplete()) {
      break;
      }
      idx++;
  }
  queueFamilyIndices = indices;
}
}