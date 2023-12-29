#include "gui.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "implot.h"
#include "swapchain.hpp"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rn {
Gui::Gui(VulkanHandler &vlkn, Window &window, const SwapChain &swapchain)
    : vlkn(vlkn), window(window) {
    createDescriptorPool();
    createContext(swapchain);
    uploadFonts();
    createFramebuffers(swapchain);
  };

Gui::~Gui() {
  vlkn.getDevice().waitIdle();
  destroyFramebuffers();
  destroyRenderPass();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  vlkn.getDevice().destroyDescriptorPool(pool);
}

void Gui::createContext(const SwapChain &swapchain) {
  uint32_t imgCount = swapchain.numberOfImages();

  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImPlot::CreateContext();
  ImPlot::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(window.get(), true);
  ImGui_ImplVulkan_InitInfo info{*vlkn.getInstance(),
                                 vlkn.getPhysDevice(),
                                 vlkn.getDevice(),
                                 vlkn.gQueueIndex(),
                                 vlkn.getGqueue(),
                                 VK_NULL_HANDLE,
                                 pool,
                                 0,
                                 2,
                                 imgCount,
                                 VK_SAMPLE_COUNT_1_BIT};

  createRenderPass(swapchain.getImageFormat());

  ImGui_ImplVulkan_Init(&info, renderPass);
}

void Gui::createDescriptorPool() {
    // allocate a humungous descriptor pool?
  // TODO: reduce size

  vk::DescriptorPoolSize poolSizes[] = {
      {vk::DescriptorType::eSampler, 1000},
      {vk::DescriptorType::eCombinedImageSampler, 1000},
      {vk::DescriptorType::eSampledImage, 1000},
      {vk::DescriptorType::eStorageImage, 1000},
      {vk::DescriptorType::eUniformTexelBuffer, 1000},
      {vk::DescriptorType::eStorageTexelBuffer, 1000},
      {vk::DescriptorType::eUniformBuffer, 1000},
      {vk::DescriptorType::eStorageBuffer, 1000},
      {vk::DescriptorType::eUniformBufferDynamic, 1000},
      {vk::DescriptorType::eStorageBufferDynamic, 1000},
      {vk::DescriptorType::eInputAttachment, 1000}};

  vk::DescriptorPoolCreateInfo createInfo(
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      1000 * IM_ARRAYSIZE(poolSizes),
      static_cast<uint32_t> IM_ARRAYSIZE(poolSizes), poolSizes);
  pool = vlkn.getDevice().createDescriptorPool(createInfo);
}

void Gui::uploadFonts() {
  ImGui_ImplVulkan_CreateFontsTexture();
  vlkn.getDevice().waitIdle();
}

void Gui::createFramebuffers(const SwapChain & swapchain) {
  framebuffers.resize(swapchain.numberOfImages());
  vk::FramebufferCreateInfo info{{},
                                 renderPass,
                                 1,
                                 {},
                                 swapchain.getExtent().width,
                                 swapchain.getExtent().height,
                                 1};
  for (size_t i = 0; i < swapchain.numberOfImages(); ++i) {
    info.setAttachments(swapchain.getImageViewPtr(i));
    framebuffers.at(i) = vlkn.getDevice().createFramebuffer(info);
  }
}

void Gui::createRenderPass(vk::Format format) {
  vk::AttachmentDescription attachment{{},
                                       format,
                                       vk::SampleCountFlagBits::e1,
                                       vk::AttachmentLoadOp::eLoad,
                                       vk::AttachmentStoreOp::eStore,
                                       vk::AttachmentLoadOp::eDontCare,
                                       vk::AttachmentStoreOp::eDontCare,
                                       vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::ePresentSrcKHR};
  vk::AttachmentReference color{0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpass{{},      vk::PipelineBindPoint::eGraphics,
                                 nullptr, color,
                                 nullptr, nullptr,
                                 nullptr};

  vk::SubpassDependency dependency{
      vk::SubpassExternal,
      0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      {},
      vk::AccessFlagBits::eColorAttachmentWrite};

  vk::RenderPassCreateInfo info{{}, attachment, subpass, dependency};

  renderPass = vlkn.getDevice().createRenderPass(info);
}

void Gui::destroyFramebuffers() {
  for (auto &buffer : framebuffers) {
    vlkn.getDevice().destroyFramebuffer(buffer);
  }
}

void Gui::destroyRenderPass() {
  vlkn.getDevice().destroyRenderPass(renderPass);
}

void Gui::recreateFramebuffers(const SwapChain &swapchain) {
  destroyFramebuffers();
  createFramebuffers(swapchain);
}

}