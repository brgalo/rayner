#include "renderer.hpp"
#include "descriptors.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "window.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include "vma.hpp"

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

void Renderer::updateCamera(float frameTime) {
  camera.updateView(frameTime);
}

void Renderer::render(vk::Buffer vertexBuffer, vk::Buffer indexBuffer,
                      size_t nIdx, RaytracingPipeline::RtConsts &RtConstsPoints,
                      RaytracingPipeline::RtConsts &RtConstsRays) {
    vlkn->getDevice().waitIdle();

  auto idx =
      swapChain.aquireNextImage(swapChain.inFlightFences.at(syncIdx),
                                swapChain.imageAvailableSemaphores.at(syncIdx));
  if (!idx.has_value()) {
    throw std::runtime_error("could get valid image to render!");
  }

  // camera
  descriptors.update(camera.getProjView(), syncIdx);
  
  auto &buffer = commandBuffers.at(syncIdx);
  buffer.begin(vk::CommandBufferBeginInfo{});
  std::array<vk::ClearValue, 2> clearVals{
      {vk::ClearDepthStencilValue{1.0f, 0},
       vk::ClearColorValue{0.01f, 0.01f, 0.01f, 1.0f}}};
  vk::RenderPassBeginInfo beginInfo{swapChain.getRenderPass(),
                                    swapChain.getFramebuffer(idx.value()),
                                    {vk::Offset2D{0, 0}, swapChain.getExtent()},
                                    clearVals};
  buffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
  /*
  buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineTri.get());
  buffer.pushConstants(pipelineTri.getLayout(),
                       vk::ShaderStageFlagBits::eVertex |
                           vk::ShaderStageFlagBits::eFragment,
                       0, sizeof(Consts), &consts);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            pipelineTri.getLayout(), 0,
                            descriptors.getSets().at(syncIdx), nullptr);
  buffer.bindVertexBuffers(0, vertexBuffer, {0});
  buffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
*/
  buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChain.getExtent().width),
                                     static_cast<float>(swapChain.getExtent().height), 0.0f, 1.0f));
  buffer.setScissor(0, vk::Rect2D(vk::Offset2D{0, 0}, swapChain.getExtent()));
  //buffer.drawIndexed(nIdx, 1, 0, 0, 0);

  // render lines
  if (getGui()->state->rayShow) {
  buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineLin.get());
  buffer.setLineWidth(5.f);
  buffer.pushConstants(pipelineLin.getLayout(),
                       vk::ShaderStageFlagBits::eVertex |
                           vk::ShaderStageFlagBits::eFragment,
                       0, sizeof(RtConstsRays), &RtConstsRays);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            pipelineLin.getLayout(), 0,
                            descriptors.getSets().at(syncIdx), nullptr);
  buffer.bindVertexBuffers(0, vertexBuffer, {0});
  buffer.draw(getGui()->state->nRays, 1, 0, 0);
  }
  // render points
  if (getGui()->state->pShow) {
  buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelinePts.get());
  buffer.pushConstants(pipelinePts.getLayout(),
                       vk::ShaderStageFlagBits::eVertex |
                           vk::ShaderStageFlagBits::eFragment,
                       0, sizeof(RtConstsPoints), &RtConstsPoints);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            pipelinePts.getLayout(), 0,
                            descriptors.getSets().at(syncIdx), nullptr);
//  buffer.bindVertexBuffers(0, vertexBuffer, {0});
  buffer.draw(getGui()->state->nPoints, 1, 0, 0);
  };

  vk::DeviceAddress temp = RtConstsRays.out;
  if (getGui()->state->hitShow) {
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelinePts.get());

  RtConstsRays.out = RtConstsRays.dir;
  buffer.pushConstants(pipelinePts.getLayout(),
                       vk::ShaderStageFlagBits::eVertex |
                           vk::ShaderStageFlagBits::eFragment,
                       0, sizeof(RtConstsRays), &RtConstsRays);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            pipelinePts.getLayout(), 0,
                            descriptors.getSets().at(syncIdx), nullptr);
  buffer.draw(getGui()->state->nRays, 1, 0, 0);
  };


  buffer.endRenderPass();
  gui->render(buffer, idx.value(), swapChain.getExtent());
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

  if (getGui()->state->pShow) {
    RtConstsRays.out = temp;
  };

  syncIdx = ++syncIdx % SwapChain::MAX_FRAMES_IN_FLIGHT;
} 
}