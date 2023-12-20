#include "descriptors.hpp"
#include "swapchain.hpp"
#include "vk_mem_alloc.h"
#include "vma.hpp"

// std
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

namespace rn {

DescriptorSet::~DescriptorSet() {
  for (size_t i = 0; i<allocs.size();++i) {
    vlkn->getVma()->destroyBuffer(allocs[i], buffers[i]);
  }
  vlkn->getDevice().destroyDescriptorSetLayout(layout);
  vlkn->getDevice().destroyDescriptorPool(pool);
}

void DescriptorSet::freeSets() {
  vlkn->getDevice().freeDescriptorSets(pool, sets);
}

void DescriptorSet::updateSets() {
  
}

void DescriptorSet::createPool() {
  vk::DescriptorPoolCreateInfo createInfo{
      {}, SwapChain::MAX_FRAMES_IN_FLIGHT, poolSize};
  pool = vlkn->getDevice().createDescriptorPool(createInfo);
}

void DescriptorSet::addBinding(uint32_t bindingNo, vk::DescriptorType type, vk::ShaderStageFlags,
                uint32_t count) {
  vk::DescriptorSetLayoutBinding binding{bindingNo, type, count};
  assert(bindings.count(bindingNo) == 0 && "Binding already taken!");
  bindings[bindingNo] = binding;
}

void DescriptorSet::createSets() {
  std::vector<vk::DescriptorSetLayoutBinding> layoutB;
  for (auto &b : bindings) {
    layoutB.push_back(b.second);
  }
  layout = vlkn->getDevice().createDescriptorSetLayout(
      vk::DescriptorSetLayoutCreateInfo{{}, layoutB});

  sets = vlkn->getDevice().allocateDescriptorSets({pool, layout});
}

void RenderDescriptors::createSets() {
  DescriptorSet::createSets();
  vk::BufferCreateInfo createInfo{
      {}, sizeof(UniformBuffer), vk::BufferUsageFlagBits::eUniformBuffer};
  allocs.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
  allocInfos.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
  VmaAllocationCreateInfo allocCreateInfo{
      VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0};
  for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
    buffers.push_back(vlkn->getVma()->createBuffer(
        allocs[i], allocInfos[i], createInfo, allocCreateInfo));
  }
}
} // namespace rn
