#pragma once

#include "window.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

#include "vk_mem_alloc.h"

namespace rn {
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class Gui;

class SwapChain {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
  SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
            Window &window);
  ~SwapChain();
  std::shared_ptr<Gui> gui;
  const vk::SwapchainKHR &getSwapchain() const {return swapchain;};

  vk::RenderPass getRenderPass() { return renderPass; };

  vk::Framebuffer getFramebuffer(size_t idx) { return framebuffers.at(idx); };
  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> imagesInFlight;
  std::vector<vk::Fence> inFlightFences;
  void resetFences();
  std::optional<uint32_t> aquireNextImage(vk::Fence fence, vk::Semaphore sema);
  void recreate();
  size_t numberOfImages() const {return imageCount;};
  vk::Extent2D getExtent() const { return extent; };
  vk::Format getImageFormat() const { return surfaceFormat.format; };
  const vk::ImageView &getImageViewPtr(size_t idx) const {
    return scImageViews.at(idx);
  };
  void setGui(std::shared_ptr<Gui> gui_) { gui = gui_; };
private:
  Window &window;
  void init();
  void createSC(std::shared_ptr<SwapChain> old);

  void createImageViews();
  void createRenderPass();
  void createDepthResources();
  void createFramebuffers();
  void createSynObjects();
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

  vk::SwapchainKHR swapchain;
  vk::RenderPass renderPass;
  std::vector<vk::Image> scImages;
  std::vector<vk::ImageView> scImageViews;
  std::vector<vk::Image> depthImages;
  std::vector<vk::ImageView> depthImageViews;
  std::vector<VmaAllocation> depthImageAlloc;
  std::vector<VmaAllocationInfo> depthImageAllocInfo;
  std::vector<vk::Framebuffer> framebuffers;

  uint32_t currImg = 0;
  std::shared_ptr<SwapChain> old;
};
} // namespace rn