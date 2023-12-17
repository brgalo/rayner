#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "swapchain.hpp"
#include <memory>
#include <vector>




namespace rn {

struct UBO {
  glm::mat4 mat;
};

class Renderer {
public:
  Renderer(std::shared_ptr<VulkanHandler> vlkn_) : vlkn(vlkn_) {
    createCommandBuffers();
  };
  
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

private:
  std::shared_ptr<VulkanHandler> vlkn;
  Window window = Window{vlkn};
  SwapChain swapChain = SwapChain(vlkn, window);

  void createCommandBuffers();
  void freeCommandBuffers();
  std::vector<vk::CommandBuffer> commandBuffers;
};
}