#pragma once
#include "descriptors.hpp"
#include "imgui.h"
#include <glm/fwd.hpp>
#include "camera.hpp"

#include "swapchain.hpp"
#include "pipeline.hpp"
#include "gui.hpp"
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
    swapChain.setGui(gui);
  };

  ~Renderer();
  bool run();
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  void render(vk::Buffer vert);
  void updateCamera(float frameTime);
private:
  std::shared_ptr<VulkanHandler> vlkn = nullptr;
  Window window = Window{vlkn};
  SwapChain swapChain = SwapChain(vlkn, window);
  RenderDescriptors descriptors;
  GraphicsPipelineTriangles pipelineTri =
      GraphicsPipelineTriangles(descriptors, swapChain.getRenderPass(), vlkn);
  GraphicsPipelineLines pipelineLin =
      GraphicsPipelineLines(descriptors, swapChain.getRenderPass(), vlkn);
  
  std::shared_ptr<Gui> gui = std::make_shared<Gui>(*vlkn, window, swapChain);

  Consts consts{};
  Camera camera{window};

  //vk::DescriptorSetLayout getConstLayout();

  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapchain();
  std::vector<vk::CommandBuffer> commandBuffers;
  uint32_t syncIdx = 0;
};
}