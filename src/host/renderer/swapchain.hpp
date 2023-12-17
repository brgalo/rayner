#pragma once

#include "window.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace rn {
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
  SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
            const Window &window);
  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;
  const vk::SwapchainKHR &getSwapchain() const {return swapChain;};


private:
  const Window &window;
  void init();
  void createSC();
  void createImageViews();
  void createRenderPass();
  void createDepthResources();
  void createFramebuffers();
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat();
  vk::PresentModeKHR chooseSwapPresentMode();
  vk::Extent2D chooseSwapExtent();
  vk::Format chooseDepthFormat();
  vk::Format depthFormat;
  SwapChainSupportDetails querySwapChainSupport();
  std::shared_ptr<VulkanHandler> vlkn;
  SwapChainSupportDetails details;
  vk::Extent2D extent;
  vk::SurfaceFormatKHR surfaceFormat;
  vk::PresentModeKHR presentMode;
  uint32_t imageCount;

  vk::SwapchainKHR swapChain;
  vk::RenderPass renderPassLines;
  vk::RenderPass renderPassTriangles;
  std::vector<vk::Image> scImages;
  std::vector<vk::ImageView> scImageViews;
  std::vector<vk::Image> depthImages;
  std::vector<vk::ImageView> depthImageViews;
  std::vector<VmaAllocation> depthImageAlloc;
  std::vector<VmaAllocationInfo> depthImageAllocInfo;
  std::vector<vk::Framebuffer> framebuffers;

  std::shared_ptr<SwapChain> oldSwapchain = nullptr;
};
} // namespace rn