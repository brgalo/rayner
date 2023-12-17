#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_enums.hpp>
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
  Pipeline();
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  virtual void bind(vk::CommandBuffer buffer);
protected:
  virtual void setConfig();
  PipelineConfigInfo configInfo;

  vk::Pipeline pipeline_;

private:
  void createShaderModule();
};

class GraphicsPipeline : protected Pipeline {
  void setConfig() override;
  void bind(vk::CommandBuffer buffer) final {
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);
  }
};

class ComputePipeline : Pipeline {

};

class RaytracingPipeline : Pipeline {};
}