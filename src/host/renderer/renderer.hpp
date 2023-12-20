#pragma once
#include "descriptors.hpp"
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
    createCommandBuffers();
  };

  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

private:
  std::shared_ptr<VulkanHandler> vlkn = nullptr;
  Window window = Window{vlkn};
  SwapChain swapChain = SwapChain(vlkn, window);
  GraphicsPipeline pipeline(vk::RenderPass pass);

  RenderDescriptors descriptors;
  vk::DescriptorSetLayout getConstLayout();

  void createCommandBuffers();
  void freeCommandBuffers();
  std::vector<vk::CommandBuffer> commandBuffers;
};
}