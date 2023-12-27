#pragma once
#include "descriptors.hpp"
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "swapchain.hpp"
#include "pipeline.hpp"
#include <memory>
#include <vector>




namespace rn {

struct Consts {
  glm::mat4 mat;
};

class Renderer {
public:
  Renderer(std::shared_ptr<VulkanHandler> vlkn_) : vlkn(vlkn_), descriptors(vlkn_){
    consts.mat = glm::mat4{1.0f};
    createCommandBuffers();

  };

  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  void render(vk::Buffer vert);
private:
  std::shared_ptr<VulkanHandler> vlkn = nullptr;
  Window window = Window{vlkn};
  SwapChain swapChain = SwapChain(vlkn, window);
  RenderDescriptors descriptors;
  GraphicsPipeline pipeline =
      GraphicsPipeline(descriptors, vlkn, swapChain.getRenderPasses());
  Consts consts{};

  //vk::DescriptorSetLayout getConstLayout();

  void createCommandBuffers();
  void freeCommandBuffers();
  void createRenderPass();
  std::vector<vk::CommandBuffer> commandBuffers;
};
}