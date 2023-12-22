#include "renderer.hpp"
#include "swapchain.hpp"
#include <memory>

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

vk::DescriptorSetLayout Renderer::getConstLayout() {
  vk::PushConstantRange range{vk::ShaderStageFlagBits::eFragment |
                                  vk::ShaderStageFlagBits::eGeometry,
                              0, sizeof(Consts)};
}
} 