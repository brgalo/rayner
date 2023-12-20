#pragma once

#include "descriptors.hpp"



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
  Pipeline();
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  void bind(vk::CommandBuffer buffer) {
    buffer.bindPipeline(bindPoint, pipeline_);
  }
protected:
  virtual void config();
  virtual void createLayout();
  PipelineConfigInfo configInfo;

  vk::Pipeline pipeline_;

private:
  const vk::PipelineBindPoint bindPoint;
  vk::PipelineLayout layout_;
};

class GraphicsPipeline : protected Pipeline {
  void config() override;
  void createLayout() override;
  RenderPushConstsData consts;
};

class ComputePipeline : Pipeline {

};

class RaytracingPipeline : Pipeline {};
}