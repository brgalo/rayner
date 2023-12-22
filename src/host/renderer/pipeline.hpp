#pragma once

#include "descriptors.hpp"
#include "vknhandler.hpp"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>



namespace rn {


struct PipelineConfigInfo {
  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

  vk::PipelineViewportStateCreateInfo viewportInfo;
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo multisampleInfo;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<vk::DynamicState> dynamicStateEnables;
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  vk::PipelineLayout pipelineLayout = nullptr;
  vk::RenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class Pipeline {
public:
  Pipeline(DescriptorSet &set_, vk::PipelineBindPoint bindP,
           std::shared_ptr<VulkanHandler> vulkn_)
      : set(set_), bindPoint(bindP), vlkn(vulkn_) {};
  ~Pipeline() {
    vlkn->getDevice().destroyPipelineLayout(layout_);
  };

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  void bind(vk::CommandBuffer buffer) {
    buffer.bindPipeline(bindPoint, pipeline_);
  }
  vk::ShaderModule createModule(const std::string &filepath);
  void destroyModule(vk::ShaderModule);

protected:
  virtual void config();
  virtual void createLayout(){};
  virtual void create();
  virtual void createRenderPass();
  PipelineConfigInfo configInfo = {};

  vk::Pipeline pipeline_;
  std::shared_ptr<VulkanHandler> vlkn;
  vk::PipelineLayout layout_;

  DescriptorSet &set;
private:
  const vk::PipelineBindPoint bindPoint;
  std::vector<char> readFile(const std::string &filepath);
};

class GraphicsPipeline : protected Pipeline {
public:
  GraphicsPipeline(DescriptorSet &set_, std::shared_ptr<VulkanHandler> vlkn,
                   std::array<vk::RenderPass,2> renderPasses)
      : Pipeline(set_, vk::PipelineBindPoint::eGraphics, vlkn) {
    triangleRenderPass = renderPasses[0];
    lineRenderPass = renderPasses[1];
    init();
  };
  ~GraphicsPipeline() {
  }
  void config() override;
  void createLayout() override;
  void init();
  RenderPushConstsData consts;
  vk::RenderPass triangleRenderPass;
  vk::RenderPass lineRenderPass;

private:
  
};

class ComputePipeline : Pipeline {

};

class RaytracingPipeline : Pipeline {};
}