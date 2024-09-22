#pragma once

#include <vulkan/vulkan.hpp>
#include "descriptors.hpp"
#include "geometryloader/geometry.hpp"
#include "vknhandler.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>


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
           std::shared_ptr<VulkanHandler> vulkn_);
  ~Pipeline() {
    vlkn->getDevice().destroyPipeline(pipeline_);
    vlkn->getDevice().destroyPipelineLayout(layout_);
  };

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  void bind(vk::CommandBuffer buffer) {
    buffer.bindPipeline(bindPoint, pipeline_);
  }
  vk::ShaderModule createModule(const std::string &filepath);
  void destroyModule(vk::ShaderModule);
  vk::Pipeline get() { return pipeline_; };
  vk::PipelineLayout getLayout() { return layout_; };

protected:
  virtual void config() = 0;
  virtual void createLayout() = 0;
  void createLayout(vk::PushConstantRange &constRange);
  PipelineConfigInfo configInfo = {};

  vk::Pipeline pipeline_;
  std::shared_ptr<VulkanHandler> vlkn;
  vk::PipelineLayout layout_;

  DescriptorSet &set;
private:
  const vk::PipelineBindPoint bindPoint;
  std::vector<char> readFile(const std::string &filepath);
};

class GraphicsPipeline : public Pipeline {
public:
  GraphicsPipeline(DescriptorSet &set_, vk::RenderPass renderPass_,
                   std::shared_ptr<VulkanHandler> vlkn);

protected:
  vk::RenderPass renderPass;
  virtual void config() override;
  void init(const std::string &vertPath, const std::string &fragPath,
            vk::PipelineVertexInputStateCreateInfo const &vertexInfo);
  virtual void createLayout() override;
  void create(vk::GraphicsPipelineCreateInfo &info);
};

class GraphicsPipelineTriangles : public GraphicsPipeline {
public:
  GraphicsPipelineTriangles(DescriptorSet &set_, vk::RenderPass renderPass_,
                            std::shared_ptr<VulkanHandler> vlkn);
  ~GraphicsPipelineTriangles() {
  }

  RenderPushConstsData consts;
  vk::RenderPass renderPass;

private:
  void config() override;
};

class GraphicsPipelineLines : public GraphicsPipeline{
public:
  GraphicsPipelineLines(DescriptorSet &set_, vk::RenderPass renderPass_,
                        std::shared_ptr<VulkanHandler> vlkn);

private:
  void config() override;
};

class GraphicsPipelinePoints : public GraphicsPipeline{
public:
  GraphicsPipelinePoints(DescriptorSet &set_, vk::RenderPass renderPass_,
                        std::shared_ptr<VulkanHandler> vlkn);

private:
  void config() override;
  void createLayout() override;
};

class ComputePipeline : Pipeline {

};

class RaytracingPipeline : public Pipeline {
public:
  RaytracingPipeline(DescriptorSet &set_, vk::PipelineBindPoint bindP,
                     std::shared_ptr<VulkanHandler> vulkn_);
  ~RaytracingPipeline();
  
  vk::StridedDeviceAddressRegionKHR rgenRegion{};
  vk::StridedDeviceAddressRegionKHR hitRegion{};
  vk::StridedDeviceAddressRegionKHR missRegion{};

  vk::ShaderModule cHit = createModule("spv/rt.rchit.spv");
  vk::ShaderModule rGen = createModule("spv/rt.rgen.spv");
  vk::ShaderModule rMiss = createModule("spv/rt.rmiss.spv");



  struct RtConsts {
    vk::DeviceAddress verts;
    vk::DeviceAddress idx;
    vk::DeviceAddress out;
  } consts;

private:
  void config() override {};
  void createLayout() override;
  uint32_t alignUp(uint32_t val, uint32_t align);

  std::vector<uint8_t> handles{};

  vk::Buffer sbtBuffer;
  VmaAllocation sbtAlloc;
  VmaAllocationInfo sbtAllocInfo;
};
}