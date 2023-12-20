#pragma once
#include "swapchain.hpp"
#include "vk_mem_alloc.h"
#include "vknhandler.hpp"

// std
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace rn {

struct RenderPushConstsData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

struct RtPushConstsData {};

class DescriptorSet {
public:
  DescriptorSet(std::shared_ptr<VulkanHandler> &vlkn_) : vlkn(vlkn_){};
  ~DescriptorSet();

  DescriptorSet(const DescriptorSet &) = delete;
  DescriptorSet &operator=(const DescriptorSet) = delete;

  vk::DescriptorSet getSet();
  void updateSets();
  void addPoolSize(vk::DescriptorType type, uint32_t count) {
    poolSize.push_back({type, count});
  };
  void addBinding(uint32_t binding, vk::DescriptorType type,
                  vk::ShaderStageFlags, uint32_t count = 1);

protected:
  virtual void createSets();
  std::shared_ptr<VulkanHandler> vlkn = nullptr;
  std::vector<vk::Buffer> buffers;
  std::vector<VmaAllocation> allocs;
  std::vector<VmaAllocationInfo> allocInfos;
  void createPool();

  void freeSets();

private:
  vk::DescriptorSetLayout getLayout();

  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;
  vk::DescriptorSetLayout layout;
  std::vector<vk::DescriptorSet> sets;
  vk::DescriptorPool pool;
  std::vector<vk::DescriptorPoolSize> poolSize;
};

class RenderDescriptors : DescriptorSet {
public:
  RenderDescriptors(std::shared_ptr<VulkanHandler> vlkn) : DescriptorSet(vlkn) {
    addPoolSize(vk::DescriptorType::eUniformBuffer, 2);
    createPool();

    addBinding(0, vk::DescriptorType::eUniformBuffer,
               vk::ShaderStageFlagBits::eVertex);
    RenderDescriptors::createSets();
  };

private:
  struct UniformBuffer {

  } ub;

  void createSets() override;
};

} // namespace rn