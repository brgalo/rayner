#include "renderer.hpp"
#include "swapchain.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace rn {
Renderer::~Renderer() {
  freeCommandBuffers();
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
  uint32_t idx = swapChain.aquireNextImage(swapChain.inFlightFences.front(),swapChain.imageAvailableSemaphores.front());
  auto &buffer = commandBuffers.front();
  buffer.begin(vk::CommandBufferBeginInfo{});
  std::array<vk::ClearValue, 2> clearVals{
      {vk::ClearDepthStencilValue{1.0f, 0},
       vk::ClearColorValue{0.01f, 0.01f, 0.01f, 1.0f}}};
  vk::RenderPassBeginInfo beginInfo{pipeline.triangleRenderPass,
                                    swapChain.getFramebuffer(),
                                    {vk::Offset2D{0, 0}, window.getExtent()},
                                    clearVals};
  buffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
  buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
  buffer.pushConstants(pipeline.getLayout(),
                       vk::ShaderStageFlagBits::eVertex |
                           vk::ShaderStageFlagBits::eFragment,
                       0, sizeof(Consts), &consts);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            pipeline.getLayout(), 0,
                            descriptors.getSets().front(), nullptr);
  buffer.bindVertexBuffers(0, vert, {0});
  buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(1000),
                                     static_cast<float>(500), 0.0f, 1.0f));
  buffer.setScissor(0, vk::Rect2D(vk::Offset2D{0, 0}, window.getExtent()));
  buffer.draw(3, 1, 0, 0);
  buffer.endRenderPass();
  buffer.end();

  if (vk::Result::eSuccess !=
      vlkn->getDevice().waitForFences(swapChain.inFlightFences.front(), VK_TRUE,
                                      UINT64_MAX)) {
    throw std::runtime_error("waited too long!");
  };

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  vk::SubmitInfo submitInfo(swapChain.imageAvailableSemaphores.front(),
                            waitDestinationStageMask, buffer,
                            swapChain.renderFinishedSemaphores.front());
  vlkn->getDevice().resetFences(swapChain.inFlightFences.front());
  vlkn->getGqueue().submit(submitInfo, swapChain.inFlightFences.front());
  uint32_t n = 0;
  vk::PresentInfoKHR presentInfo{swapChain.renderFinishedSemaphores.front(),
                                 swapChain.getSwapchain(), idx};

  auto res = vlkn->getGqueue().presentKHR(presentInfo);
  vlkn->getDevice().waitIdle();
}S
}
 