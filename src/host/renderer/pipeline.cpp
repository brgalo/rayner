#include "pipeline.hpp"
#include "descriptors.hpp"
#include "geometryloader/geometry.hpp"
#include "vknhandler.hpp"
#include <array>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace rn {

Pipeline::Pipeline(DescriptorSet &set_, vk::PipelineBindPoint bindP,
                   std::shared_ptr<VulkanHandler> vulkn_)
    : set(set_), bindPoint(bindP), vlkn(vulkn_) {}

GraphicsPipeline::GraphicsPipeline(DescriptorSet &set_, vk::RenderPass renderPass_,
                   std::shared_ptr<VulkanHandler> vlkn)
      : Pipeline(set_, vk::PipelineBindPoint::eGraphics, vlkn){
  renderPass = renderPass_;
  GraphicsPipeline::config();
};

GraphicsPipelineTriangles::GraphicsPipelineTriangles(DescriptorSet &set_,
                            vk::RenderPass renderPass_,
                            std::shared_ptr<VulkanHandler> vlkn)
      : GraphicsPipeline(set_, renderPass_, vlkn) {
        GraphicsPipelineTriangles::config();
        init("spv/tri.vert.spv", "spv/tri.frag.spv");
};

GraphicsPipelineLines::GraphicsPipelineLines(
    DescriptorSet &set_, vk::RenderPass renderPass_,
    std::shared_ptr<VulkanHandler> vlkn)
    : GraphicsPipeline(set_, renderPass_, vlkn) {
        GraphicsPipelineLines::config();
        init("spv/lin.vert.spv","spv/lin.frag.spv");
};

GraphicsPipelinePoints::GraphicsPipelinePoints(
    DescriptorSet &set_, vk::RenderPass renderPass_,
    std::shared_ptr<VulkanHandler> vlkn)
    : GraphicsPipeline(set_, renderPass_, vlkn) {
        GraphicsPipelinePoints::config();
        init("spv/pts.vert.spv", "spv/pts.frag.spv");
};

RaytracingPipeline::RaytracingPipeline(DescriptorSet &set_,
                                       vk::PipelineBindPoint bindP,
                                       std::shared_ptr<VulkanHandler> vulkn_)
    : Pipeline(set_, bindP, vulkn_) {
      RaytracingPipeline::createLayout();

      vk::ShaderModule cHit = createModule("spv/rt.rchit.spv");
      vk::ShaderModule rGen = createModule("spv/rt.rgen.spv");
      vk::ShaderModule rMiss = createModule("spv/rt.rmiss.spv");

      std::array<vk::PipelineShaderStageCreateInfo, 3> shaderStages{
          {{{}, vk::ShaderStageFlagBits::eRaygenKHR, rGen, "main"},
           {{}, vk::ShaderStageFlagBits::eClosestHitKHR, cHit, "main"},
           {{}, vk::ShaderStageFlagBits::eMissKHR, rMiss, "main"}}};

      std::array<vk::RayTracingShaderGroupCreateInfoKHR, 3> rtsgci{
          {{vk::RayTracingShaderGroupTypeKHR::eGeneral, 0, vk::ShaderUnusedKHR,
            vk::ShaderUnusedKHR, vk::ShaderUnusedKHR},
           {vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,
            vk::ShaderUnusedKHR, 1, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR},
           {vk::RayTracingShaderGroupTypeKHR::eGeneral, 2, vk::ShaderUnusedKHR,
            vk::ShaderUnusedKHR, vk::ShaderUnusedKHR}}};
      vk::RayTracingPipelineCreateInfoKHR rtpci{{}, shaderStages, rtsgci};
      rtpci.layout = layout_;

      pipeline_ = vlkn->getDevice()
                      .createRayTracingPipelinesKHR(VK_NULL_HANDLE,
                                                    VK_NULL_HANDLE, rtpci)
                      .value.front();
      
        

      destroyModule(cHit);
      destroyModule(rGen);
      destroyModule(rMiss);
  
};



void GraphicsPipeline::config() {
  // will be set dynamically!
  configInfo.viewportInfo.viewportCount = 1;
  configInfo.viewportInfo.pViewports = nullptr;
  configInfo.viewportInfo.scissorCount = 1;
  configInfo.viewportInfo.pScissors = nullptr;

  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
  configInfo.rasterizationInfo.lineWidth = 1.0f;
  configInfo.rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
  configInfo.rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
  configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

  configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
  configInfo.multisampleInfo.minSampleShading = 1.0f;
  configInfo.multisampleInfo.pSampleMask = nullptr;
  configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
  configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

  configInfo.colorBlendAttachment.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
   configInfo.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
  configInfo.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
  configInfo.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  configInfo.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  configInfo.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy;
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f;
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = vk::StencilOpState();
  configInfo.depthStencilInfo.back = vk::StencilOpState();

  configInfo.dynamicStateEnables.reserve(10);
  configInfo.dynamicStateEnables = {vk::DynamicState::eViewport,
                                    vk::DynamicState::eScissor};
  configInfo.dynamicStateInfo =
      vk::PipelineDynamicStateCreateInfo{{}, configInfo.dynamicStateEnables};
}

void GraphicsPipelineTriangles::config() {
  configInfo.inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);
  configInfo.inputAssemblyInfo.setTopology(
      vk::PrimitiveTopology::eTriangleList);
}


void GraphicsPipelineLines::config() {
  // change primitive type!
  configInfo.inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eLineList);

  configInfo.dynamicStateEnables.push_back(vk::DynamicState::eLineWidth);
   configInfo.dynamicStateInfo =
      vk::PipelineDynamicStateCreateInfo{{}, configInfo.dynamicStateEnables};
}

void GraphicsPipelinePoints::config() {
   // change primitive type!
   configInfo.inputAssemblyInfo.setTopology(vk::PrimitiveTopology::ePointList);
}

void GraphicsPipeline::create(vk::GraphicsPipelineCreateInfo &info) {
  auto res = vlkn->getDevice().createGraphicsPipeline(nullptr, info);
  if (res.result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create Pipeline!");
  }
  pipeline_ = res.value;
}

void RaytracingPipeline::createLayout() {
  vk::PipelineLayoutCreateInfo layoutInfo{{},1,&set.getLayout()};
  layout_ = vlkn->getDevice().createPipelineLayout(layoutInfo);
};

void GraphicsPipeline::init(const std::string &vertPath,
                            const std::string &fragPath) {
  createLayout();

  // shader modules
  vk::ShaderModule vert = createModule(vertPath);
  vk::ShaderModule frag = createModule(fragPath);

  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
      {{{}, vk::ShaderStageFlagBits::eVertex, vert, "main"},
       {{}, vk::ShaderStageFlagBits::eFragment, frag, "main"}}};
  auto inputDesc = GeometryHandler::getInputDescription();
  auto inputAttr = GeometryHandler::getAttributeDescription();
  vk::PipelineVertexInputStateCreateInfo vertexInfo{{}, inputDesc, inputAttr};

  vk::GraphicsPipelineCreateInfo createInfo{{},
                                            shaderStages,
                                            &vertexInfo,
                                            &configInfo.inputAssemblyInfo,
                                            nullptr,
                                            &configInfo.viewportInfo,
                                            &configInfo.rasterizationInfo,
                                            &configInfo.multisampleInfo,
                                            &configInfo.depthStencilInfo,
                                            &configInfo.colorBlendInfo,
                                            &configInfo.dynamicStateInfo,
                                            layout_,
                                            renderPass,
                                            configInfo.subpass};
  create(createInfo);
  destroyModule(vert);
  destroyModule(frag);
}

vk::ShaderModule Pipeline::createModule(const std::string &filepath) {
  return vlkn->createShaderModule(readFile(filepath));
}

void Pipeline::destroyModule(vk::ShaderModule module) {
  vlkn->destroyShaderModule(module);
}

std::vector<char> Pipeline::readFile(const std::string &filepath) {
  std::ifstream shadercode{filepath, std::ios::ate | std::ios::binary};
  if (!shadercode.is_open()) {
    std::runtime_error("failed to open file: " + filepath);
  }
  size_t size = static_cast<size_t>(shadercode.tellg());
  std::vector<char> buffer(size);

  shadercode.seekg(0);
  shadercode.read(buffer.data(), size);
  shadercode.close();
  return buffer;  
};

void GraphicsPipeline::createLayout() {
  vk::PushConstantRange constRange{vk::ShaderStageFlagBits::eVertex |
                            vk::ShaderStageFlagBits::eFragment,
                        0, sizeof(RenderPushConstsData)};

  std::vector<vk::DescriptorSetLayout> layouts{set.getLayout()};
  vk::PipelineLayoutCreateInfo createInfo{{}, layouts, constRange};
  layout_ = vlkn->getDevice().createPipelineLayout(createInfo);
};

}