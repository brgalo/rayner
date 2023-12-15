#include "swapchain.hpp"
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace rn {
SwapChain::SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
                     const Window &window_) : window(window_) {
  vlkn = vulkanHandler;
  create(window.getSurface());
  details.capabilities =
      vlkn->getPhysDevice().getSurfaceCapabilitiesKHR(window.getSurface());
  details.formats = vlkn->getPhysDevice().getSurfaceFormatsKHR(window.getSurface());
  if (details.formats.empty())
    throw std::runtime_error("no surface formats available!");
  details.presentModes =
      vlkn->getPhysDevice().getSurfacePresentModesKHR(window.getSurface());
};

SwapChain::~SwapChain() {}

void SwapChain::create(const vk::SurfaceKHR &surface) {
  querySwapChainSupport();
  surfaceFormat = chooseSwapSurfaceFormat();
  presentMode = chooseSwapPresentMode();
  extent = chooseSwapExtent();
  imageCount = details.capabilities.minImageCount + 1;
  if (details.capabilities.minImageCount > 0 &&
      imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
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