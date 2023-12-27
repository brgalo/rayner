#include "swapchain.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "vma.hpp"

namespace rn {
SwapChain::SwapChain(std::shared_ptr<VulkanHandler> vulkanHandler,
                     const Window &window_)
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
  vlkn->getDevice().destroyRenderPass(renderPassTriangles);

  // destroy depth resources
  for (size_t i = 0; i < imageCount; ++i) {
    vlkn->getVma()->destroyImage(depthImages[i], depthImageAlloc[i]);
    vlkn->getDevice().destroyImageView(depthImageViews[i]);
  }

  // destroy swapchain and resources
  for (auto &iv : scImageViews) {
    vlkn->getDevice().destroyImageView(iv);
  }
  vlkn->getDevice().destroySwapchainKHR(swapChain);
}

void SwapChain::init() {
  createSC();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSynObjects();
}

void SwapChain::createSC() {
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
      oldSwapchain == nullptr ? VK_NULL_HANDLE : oldSwapchain->getSwapchain();

  vk::SwapchainCreateInfoKHR createInfo(
      {}, window.getSurface(), imageCount, surfaceFormat.format,
      surfaceFormat.colorSpace, extent, 1,
      vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, {},
      nullptr, details.capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, true, tempSwap);
  swapChain = vlkn->getDevice().createSwapchainKHR(createInfo);
  scImages = vlkn->getDevice().getSwapchainImagesKHR(swapChain);
  imageCount = scImages.size();
}

void SwapChain::createImageViews() {
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

  vk::AttachmentDescription colorAttachment(
      {}, depthFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal );
  attDescr[0] = colorAttachment;

  colorAttachment.setFormat(surfaceFormat.format);
  colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
  colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  attDescr[1] = colorAttachment;

  vk::AttachmentReference depthRef{
      0, vk::ImageLayout::eDepthStencilAttachmentOptimal};
  vk::AttachmentReference colorRef{1, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpassDescr{
      {}, vk::PipelineBindPoint::eGraphics, {}, colorRef, {}, &depthRef};
  renderPassTriangles = vlkn->getDevice().createRenderPass(
      vk::RenderPassCreateInfo({}, attDescr, subpassDescr));
}

void SwapChain::createDepthResources() {
  depthImages.reserve(imageCount);
  depthImageViews.reserve(imageCount);
  depthImageAlloc.reserve(imageCount);
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
  framebuffers.reserve(imageCount);
  vk::FramebufferCreateInfo info{
      {}, renderPassTriangles, {}, extent.width, extent.height, 1};
  for (size_t i = 0; i < imageCount; ++i) {
    std::vector<vk::ImageView> attachments = {depthImageViews[i],
                                              scImageViews[i]};
    framebuffers.push_back(
        vlkn->getDevice().createFramebuffer(info.setAttachments(attachments)));
  }
}

void SwapChain::createSynObjects() {
  imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
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

uint32_t SwapChain::aquireNextImage(vk::Fence fence, vk::Semaphore sema) {
  auto disc = vlkn->getDevice().waitForFences(fence, vk::True, std::numeric_limits<uint64_t>::max());
  auto res = vlkn->getDevice().acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), sema,
                                                   VK_NULL_HANDLE, &currImg);
  if (res == vk::Result::eErrorOutOfDateKHR) {
    // Todo!
  }
  return currImg;
}
} // namespace rn