#include "gui.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "implot.h"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace rn {

// from imgui_demo.cpp
// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Gui::oriMenu() {
  const char* items[] = {"Tri 1", "Tri 2", "Tri 3"};
  static int current_item = 0;
  static int nPoints = 100;
  ImGui::Combo("Triangle", &current_item, &State::itemGetter,
               triangleNames->data(), triangleNames->size());
  ImGui::DragInt("Number of points to sample", &nPoints, 1, 0, 1000);
  if (ImGui::Button("Launch")) {
    state->currTri = current_item;
    state->nPoints = nPoints;
    state->pLaunch = true;
    state->pShow = true;
  }
}

void Gui::rayMenu() {

  static int current_item = 0;
  static int nRays = 100;
  ImGui::Combo("Triangle", &current_item, &State::itemGetter,
               triangleNames->data(), triangleNames->size());
  ImGui::DragInt("Number of rays to launch", &nRays, 1, 0, 1000);
  if (ImGui::Button("Launch")) {
    state->currTri = current_item;
    state->nRays = nRays;
    state->rLaunch = true;
    state->rShow = true;
  }
};


Gui::Gui(VulkanHandler &vlkn, Window &window, const SwapChain &swapchain,std::shared_ptr<std::vector<std::string>> triangleNames_)
    : vlkn(vlkn), window(window), triangleNames(triangleNames_) {
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

void Gui::gui() {
  static int e = 0;
  ImGui::ShowDemoWindow();

  ImGui::RadioButton("Sample Origins", &e, 0);
  ImGui::SameLine();
  ImGui::RadioButton("Trace Rays", &e, 1);
  ImGui::SameLine();
  ImGui::RadioButton("C", &e, 2);
  ImGui::SameLine();
  HelpMarker("Switch between tracing modes\n"\
             "A = show randomly sampled origins\n"\
             "B = show hit points on the triangles");

  if(e == 0) {
    oriMenu();
  }
  if(e == 1) {
    rayMenu();
  }
  if(e == 2) {
    ImGui::Text("C");
  }

  ImGui::Checkbox("Show Oris", &state->pShow);
  ImGui::SameLine();
  ImGui::Checkbox("Show Rays", &state->rShow);
}

void Gui::render(vk::CommandBuffer &buffer, uint32_t idx, vk::Extent2D extent) {
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  gui();

  ImGui::Render();

  vk::RenderPassBeginInfo info{
      renderPass, framebuffers.at(idx), {{0, 0}, extent}};
  buffer.beginRenderPass(info, vk::SubpassContents::eInline);
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
  buffer.endRenderPass();
}

void Gui::createContext(const SwapChain &swapchain) {
  uint32_t imgCount = swapchain.numberOfImages();

  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImPlot::CreateContext();
  ImPlot::StyleColorsDark();

  createRenderPass(swapchain.getImageFormat());
  
  ImGui_ImplGlfw_InitForVulkan(window.get(), true);
  ImGui_ImplVulkan_InitInfo info{
    *vlkn.getInstance(), vlkn.getPhysDevice(), vlkn.getDevice(),
        vlkn.gQueueIndex(), vlkn.getGqueue(),
                                 pool,
                                 renderPass,
                                 2,
                                 imgCount,
                                 VK_SAMPLE_COUNT_1_BIT};



  ImGui_ImplVulkan_Init(&info);
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
  vk::FramebufferCreateInfo info{
    {}, renderPass, 1, {}, swapchain.getExtent().width,
        swapchain.getExtent().height,1};
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