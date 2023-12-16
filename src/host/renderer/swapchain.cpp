#include "swapchain.hpp"
#include "window.hpp"
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rn {
SwapChain::SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
                     const Window &window_) : window(window_) {
  vlkn = vulkanHandler;
  createSC(window.getSurface());
  details.capabilities =
      vlkn->getPhysDevice().getSurfaceCapabilitiesKHR(window.getSurface());
  details.formats = vlkn->getPhysDevice().getSurfaceFormatsKHR(window.getSurface());
  if (details.formats.empty())
    throw std::runtime_error("no surface formats available!");
  details.presentModes =
      vlkn->getPhysDevice().getSurfacePresentModesKHR(window.getSurface());
};

SwapChain::~SwapChain() {
  for (auto &iv : imageViews) {
    vlkn->getDevice().destroyImageView(iv);
  }
  vlkn->getDevice().destroySwapchainKHR(swapChain);
}

void SwapChain::createSC(const vk::SurfaceKHR &surface) {
  querySwapChainSupport();
  surfaceFormat = chooseSwapSurfaceFormat();
  presentMode = chooseSwapPresentMode();
  extent = chooseSwapExtent();
  imageCount = details.capabilities.minImageCount + 1;
  if (details.capabilities.minImageCount > 0 &&
      imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
  }
  auto tempSwap =
      oldSwapchain == nullptr ? VK_NULL_HANDLE : oldSwapchain->getSwapchain();

  vk::SwapchainCreateInfoKHR createInfo(
      {}, window.getSurface(), imageCount, surfaceFormat.format,
      surfaceFormat.colorSpace, extent, 1,
      vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive,
      {}, nullptr, details.capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, true, tempSwap);
  swapChain = vlkn->getDevice().createSwapchainKHR(createInfo);
  scImages = vlkn->getDevice().getSwapchainImagesKHR(swapChain);
}

void SwapChain::createIV() {
  imageViews.reserve(scImages.size());
  vk::ImageViewCreateInfo createInfo({}, {}, vk::ImageViewType::e2D, surfaceFormat.format,
                          {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  for (auto &i : scImages) {
        createInfo.image = i;
        imageViews.push_back(vlkn->getDevice().createImageView(createInfo));
  }
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat() {
  for (const auto &availableFormat : details.formats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  return details.formats[0];
}
vk::PresentModeKHR SwapChain::chooseSwapPresentMode() {
  for (const auto &availablePresentMode : details.presentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      //  std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo; // guaranteed by standard
}

vk::Extent2D SwapChain::chooseSwapExtent() {
  if (details.capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return details.capabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = window.getExtent();
    actualExtent.width =
        std::max(details.capabilities.minImageExtent.width,
                 std::min(details.capabilities.maxImageExtent.width,
                          actualExtent.width));
    actualExtent.height =
        std::max(details.capabilities.minImageExtent.height,
                 std::min(details.capabilities.maxImageExtent.height,
                          actualExtent.height));
    return actualExtent;
  }
}

SwapChainSupportDetails SwapChain::querySwapChainSupport() {
  details.capabilities =
      vlkn->getPhysDevice().getSurfaceCapabilitiesKHR(window.getSurface());
  details.formats = vlkn->getPhysDevice().getSurfaceFormatsKHR(window.getSurface());
  if (details.formats.empty())
    throw std::runtime_error("no surface formats available!");
  details.presentModes =
      vlkn->getPhysDevice().getSurfacePresentModesKHR(window.getSurface());
  return details;
}
} // namespace rn