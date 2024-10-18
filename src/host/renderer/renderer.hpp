#pragma once
#include "descriptors.hpp"
#include "geometryloader/geometry.hpp"
#include "imgui.h"
#include <cstddef>
#include <glm/fwd.hpp>
#include "camera.hpp"

#include "swapchain.hpp"
#include "pipeline.hpp"
#include "gui.hpp"
#include <memory>
#include <sys/types.h>
#include <vector>
#include "raytracer/raytracer.hpp"




namespace rn {

struct Consts {
  glm::mat4 mat;
};


class Renderer {
public:
  Renderer(std::shared_ptr<VulkanHandler> vlkn_, GeometryHandler &geom_);

  ~Renderer();
  bool run();
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  void render(vk::Buffer vertexBuffer, vk::Buffer indexBuffer, size_t nIdx,
              RaytracingPipeline::RtConsts &rtConstsPoints,
              RaytracingPipeline::RtConsts &rtConstsRays);
  void updateCamera(float frameTime);
  std::shared_ptr<Gui> getGui() { return gui; };
private:
  std::shared_ptr<VulkanHandler> vlkn = nullptr;
  Window window = Window{vlkn};
  SwapChain swapChain = SwapChain(vlkn, window);
  RenderDescriptors descriptors;
  GraphicsPipelineTriangles pipelineTri =
      GraphicsPipelineTriangles(descriptors, swapChain.getRenderPass(), vlkn);
  GraphicsPipelineLines pipelineLin =
      GraphicsPipelineLines(descriptors, swapChain.getRenderPass(), vlkn);
  GraphicsPipelinePoints pipelinePts =
      GraphicsPipelinePoints(descriptors, swapChain.getRenderPass(), vlkn);

  std::shared_ptr<Gui> gui = nullptr;

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