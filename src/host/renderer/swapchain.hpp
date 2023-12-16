#pragma once

#include "../vknhandler/vknhandler.hpp"
#include "window.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rn {
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain {
public:
  SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
            const Window &window);
  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;
  const vk::SwapchainKHR &getSwapchain() const {return swapChain;};


private:
  const Window& window;
  void createSC(const vk::SurfaceKHR &surface);
  void createIV();
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat();
  vk::PresentModeKHR chooseSwapPresentMode();
  vk::Extent2D chooseSwapExtent();
  SwapChainSupportDetails querySwapChainSupport();
  std::shared_ptr<VulkanHandler> vlkn;
  SwapChainSupportDetails details;
  vk::Extent2D extent;
  vk::SurfaceFormatKHR surfaceFormat;
  vk::PresentModeKHR presentMode;
  uint32_t imageCount;

  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> scImages;
  std::vector<vk::ImageView> imageViews;
  std::shared_ptr<SwapChain> oldSwapchain = nullptr;
};
} // namespace rn