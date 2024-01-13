#include "swapchain.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "GLFW/glfw3.h"
#include "vma.hpp"
#include "gui.hpp"

namespace rn {
SwapChain::SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
                     Window &window_)
    : window(window_) {
  vlkn = vulkanHandler;
  details.capabilities =
      vlkn->getPhysDevice().getSurfaceCapabilitiesKHR(window.getSurface());
  details.formats =
      vlkn->getPhysDevice().getSurfaceFormatsKHR(window.getSurface());
  if (details.formats.empty())
    throw std::runtime_error("no surface formats available!");
  details.presentModes =
      vlkn->getPhysDevice().getSurfacePresentModesKHR(window.getSurface());
  init();
};

SwapChain::~SwapChain() {
  // destroy sync resources
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vlkn->getDevice().destroySemaphore(imageAvailableSemaphores[i]);
    vlkn->getDevice().destroySemaphore(renderFinishedSemaphores[i]);
    vlkn->getDevice().destroyFence(inFlightFences[i]);
  }

  // destroy frambuffer
  for (auto &f : framebuffers) {
    vlkn->getDevice().destroyFramebuffer(f);
  }

  // subpass
  vlkn->getDevice().destroyRenderPass(renderPass);

  // destroy depth resources
  for (size_t i = 0; i < imageCount; ++i) {
    vlkn->getVma()->destroyImage(depthImages[i], depthImageAlloc[i]);
    vlkn->getDevice().destroyImageView(depthImageViews[i]);
  }

  // destroy swapchain and resources
  for (auto &iv : scImageViews) {
    vlkn->getDevice().destroyImageView(iv);
  }
  vlkn->getDevice().destroySwapchainKHR(swapchain);
}

void SwapChain::init() {
  createSC(std::move(old));
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSynObjects();
}

void SwapChain::createSC(std::shared_ptr<SwapChain> old) {
  querySwapChainSupport();
  surfaceFormat = chooseSwapSurfaceFormat();
  presentMode = chooseSwapPresentMode();
  extent = chooseSwapExtent();
  depthFormat = chooseDepthFormat();

  imageCount = details.capabilities.minImageCount + 1;
  if (details.capabilities.minImageCount > 0 &&
      imageCount > details.capabilities.maxImageCount) {
    imageCount = details.capabilities.maxImageCount;
  }
  auto tempSwap =
      old == nullptr ? VK_NULL_HANDLE : old->getSwapchain();

  vk::SwapchainCreateInfoKHR createInfo(
      {}, window.getSurface(), imageCount, surfaceFormat.format,
      surfaceFormat.colorSpace, extent, 1,
      vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, {},
      nullptr, details.capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, true, tempSwap);
  swapchain = vlkn->getDevice().createSwapchainKHR(createInfo);
  scImages = vlkn->getDevice().getSwapchainImagesKHR(swapchain);
  imageCount = scImages.size();
}

void SwapChain::createImageViews() {
  scImageViews.clear();
  scImageViews.reserve(imageCount);
  vk::ImageViewCreateInfo createInfo(
      {}, {}, vk::ImageViewType::e2D, surfaceFormat.format, {},
      {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  for (auto &i : scImages) {
    createInfo.image = i;
    scImageViews.push_back(vlkn->getDevice().createImageView(createInfo));
  }
}

void SwapChain::createRenderPass() {
  std::array<vk::AttachmentDescription, 2> attDescr;

  vk::AttachmentDescription depthAttachment(
      {}, depthFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal );
  attDescr[0] = depthAttachment;

vk::AttachmentDescription colorAttachment(
    {}, surfaceFormat.format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
    vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
    vk::ImageLayout::eColorAttachmentOptimal);
  attDescr[1] = colorAttachment;

  vk::AttachmentReference depthRef{
      0, vk::ImageLayout::eDepthStencilAttachmentOptimal};
  vk::AttachmentReference colorRef{1, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpassDescr{
      {}, vk::PipelineBindPoint::eGraphics, {}, colorRef, {}, &depthRef};
  renderPass = vlkn->getDevice().createRenderPass(
      vk::RenderPassCreateInfo({}, attDescr, subpassDescr));
}

void SwapChain::createDepthResources() {
  depthImages.clear();
  depthImages.reserve(imageCount);
  depthImageViews.clear();
  depthImageViews.reserve(imageCount);
  depthImageAlloc.clear();
  depthImageAlloc.reserve(imageCount);
  depthImageAllocInfo.clear();
  depthImageAllocInfo.reserve(imageCount);

  vk::ImageCreateInfo imageInfo(
      {}, vk::ImageType::e2D, depthFormat, vk::Extent3D{extent, 1}, 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eDepthStencilAttachment,
      vk::SharingMode::eExclusive);

  vk::ImageViewCreateInfo viewInfo(
      {}, nullptr, vk::ImageViewType::e2D, depthFormat, {},
      {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});

  VmaAllocationCreateInfo createInfo{};
  createInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
  for (uint32_t i = 0; i < imageCount; ++i) {
    VmaAllocation imgAlloc{};
    VmaAllocationInfo allocInfo{};
    depthImages.push_back(
        vlkn->getVma()->creatDepthImage(imgAlloc, allocInfo, imageInfo));
    depthImageAllocInfo.push_back(allocInfo);
    depthImageAlloc.push_back(imgAlloc);
    viewInfo.setImage(depthImages.back());
    depthImageViews.push_back(vlkn->getDevice().createImageView(viewInfo));
  }
}

void SwapChain::createFramebuffers() {
  framebuffers.clear();
  framebuffers.reserve(imageCount);
  extent = window.getExtent();
  vk::FramebufferCreateInfo info {
      {}, renderPass, {}, extent.width, extent.height, 1};
  for (size_t i = 0; i < imageCount; ++i) {
    std::vector<vk::ImageView> attachments = {depthImageViews[i],
                                              scImageViews[i]};
    framebuffers.push_back(
        vlkn->getDevice().createFramebuffer(info.setAttachments(attachments)));
  }
}

void SwapChain::createSynObjects() {
  imageAvailableSemaphores.clear();
  imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.clear();
  renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.clear();
  inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.clear();
  imagesInFlight.resize(imageCount, VK_NULL_HANDLE);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    imageAvailableSemaphores.push_back(
        vlkn->getDevice().createSemaphore(vk::SemaphoreCreateInfo()));
    renderFinishedSemaphores.push_back(
        vlkn->getDevice().createSemaphore(vk::SemaphoreCreateInfo()));
    inFlightFences.push_back(vlkn->getDevice().createFence(
        vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
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

vk::Format SwapChain::chooseDepthFormat() {
  std::vector<vk::Format> formats{vk::Format::eD32Sfloat,
                                  vk::Format::eD32SfloatS8Uint,
                                  vk::Format::eD24UnormS8Uint};
  vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
  vk::FormatFeatureFlagBits flags =
      vk::FormatFeatureFlagBits::eDepthStencilAttachment;
  for (auto f : formats) {
    vk::FormatProperties props = vlkn->getPhysDevice().getFormatProperties(f);
    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & flags) == flags) {
      return f;
    } else if (tiling == vk::ImageTiling::eOptimal &&
               (props.optimalTilingFeatures & flags) == flags) {
      return f;
    }
  }
  throw std::runtime_error("failed to find supported depth format!");
}

SwapChainSupportDetails SwapChain::querySwapChainSupport() {
  details.capabilities =
      vlkn->getPhysDevice().getSurfaceCapabilitiesKHR(window.getSurface());
  details.formats =
      vlkn->getPhysDevice().getSurfaceFormatsKHR(window.getSurface());
  if (details.formats.empty())
    throw std::runtime_error("no surface formats available!");
  details.presentModes =
      vlkn->getPhysDevice().getSurfacePresentModesKHR(window.getSurface());
  return details;
}

std::optional<uint32_t> SwapChain::aquireNextImage(vk::Fence fence, vk::Semaphore sema) {
  auto disc = vlkn->getDevice().waitForFences(fence, vk::True, std::numeric_limits<uint64_t>::max());
  auto res = vlkn->getDevice().acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), sema,
                                                   VK_NULL_HANDLE, &currImg);
  if (res == vk::Result::eErrorOutOfDateKHR) {
    return std::optional<uint32_t>(std::nullopt);
  }
  return currImg;
}

void SwapChain::recreate() {
  bool hasChanged = false;
  extent = window.getExtent();
  // dont render while window is minimized!
  while (extent.width == 0 || extent.height == 0) {
    extent = window.getExtent();
    glfwWaitEvents();
  }

  vlkn->getDevice().waitIdle();
  if (swapchain == nullptr) {

  } else {
    {
    hasChanged = true;
    old = std::make_shared<SwapChain>(*this);
    init();
    }

    gui->recreateFramebuffers(*this);
  }
  window.resetResizedFlag();
}
} // namespace rn