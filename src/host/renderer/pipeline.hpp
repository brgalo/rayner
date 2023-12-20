#pragma once

#include "descriptors.hpp"
#include "vknhandler.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>



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
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  void bind(vk::CommandBuffer buffer) {
    buffer.bindPipeline(bindPoint, pipeline_);
  }

protected:
  virtual void config();
  virtual void createLayout();
  PipelineConfigInfo configInfo = {};

  vk::Pipeline pipeline_;
  std::shared_ptr<VulkanHandler> vlkn;
  vk::PipelineLayout layout_;

  DescriptorSet &set;
private:
  const vk::PipelineBindPoint bindPoint;
};

class GraphicsPipeline : protected Pipeline {
public:
  GraphicsPipeline(DescriptorSet &set_, std::shared_ptr<VulkanHandler> vlkn)
      : Pipeline(set_, vk::PipelineBindPoint::eGraphics, vlkn) {
    init();
    };
  void config() override;
  void createLayout() override;
  void init();
  RenderPushConstsData consts;
};

class ComputePipeline : Pipeline {

};

class RaytracingPipeline : Pipeline {};
}