#include "renderer.hpp"
#include "descriptors.hpp"
#include "swapchain.hpp"
#include "window.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <iostream>

namespace rn {
Renderer::~Renderer() {
  freeCommandBuffers();
}

bool Renderer::run() {
  // poll inputs
  window.poll();
  if (window.shouldClose())
    return false;
  if (window.wasResized()) {
    swapChain.recreate();
  }
  return true;
}

void Renderer::createCommandBuffers() {
  commandBuffers =
      vlkn->getDevice().allocateCommandBuffers(vk::CommandBufferAllocateInfo{
          vlkn->getGpool(), vk::CommandBufferLevel::ePrimary,
          SwapChain::MAX_FRAMES_IN_FLIGHT});
}

void Renderer::freeCommandBuffers() {
  vlkn->getDevice().freeCommandBuffers(vlkn->getGpool(), commandBuffers);
}

void Renderer::render(vk::Buffer vert) {
  if (window.wasResized())
    swapChain.recreate();
  auto idx =
      swapChain.aquireNextImage(swapChain.inFlightFences.at(syncIdx),
                                swapChain.imageAvailableSemaphores.at(syncIdx));
  if (!idx.has_value()) {
    std::cout << "couldn't render, try again!" << std::endl;
  }
  auto &buffer = commandBuffers.at(syncIdx);
  buffer.begin(vk::CommandBufferBeginInfo{});
  std::array<vk::ClearValue, 2> clearVals{
      {vk::ClearDepthStencilValue{1.0f, 0},
       vk::ClearColorValue{0.01f, 0.01f, 0.01f, 1.0f}}};
  vk::RenderPassBeginInfo beginInfo{swapChain.getRenderPasses().front(),
                                    swapChain.getFramebuffer(idx.value()),
                                    {vk::Offset2D{0, 0}, swapChain.getExtent()},
                                    clearVals};
  buffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
  buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
  buffer.pushConstants(pipeline.getLayout(),
                       vk::ShaderStageFlagBits::eVertex |
                           vk::ShaderStageFlagBits::eFragment,
                       0, sizeof(Consts), &consts);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            pipeline.getLayout(), 0,
                            descriptors.getSets().at(syncIdx), nullptr);
  buffer.bindVertexBuffers(0, vert, {0});
  buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChain.getExtent().width),
                                     static_cast<float>(swapChain.getExtent().height), 0.0f, 1.0f));
  buffer.setScissor(0, vk::Rect2D(vk::Offset2D{0, 0}, swapChain.getExtent()));
  buffer.draw(3, 1, 0, 0);
  buffer.endRenderPass();
  buffer.end();

  if (vk::Result::eSuccess !=
      vlkn->getDevice().waitForFences(swapChain.inFlightFences.at(syncIdx), VK_TRUE,
                                      UINT64_MAX)) {
    throw std::runtime_error("waited too long!");
  };

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  vk::SubmitInfo submitInfo(swapChain.imageAvailableSemaphores.at(syncIdx),
                            waitDestinationStageMask, buffer,
                            swapChain.renderFinishedSemaphores.at(syncIdx));
  vlkn->getDevice().resetFences(swapChain.inFlightFences.at(syncIdx));
  
  vlkn->getGqueue().submit(submitInfo, swapChain.inFlightFences.at(syncIdx));

  vk::PresentInfoKHR presentInfo{swapChain.renderFinishedSemaphores.at(syncIdx),
                                 swapChain.getSwapchain(), idx.value()};

  auto res = vlkn->getGqueue().presentKHR(presentInfo);
    if (res == vk::Result::eErrorOutOfDateKHR) {
    swapChain.recreate();
    return;
  }
  vlkn->getDevice().waitIdle();

  syncIdx = ++syncIdx % SwapChain::MAX_FRAMES_IN_FLIGHT;
} 
}
 