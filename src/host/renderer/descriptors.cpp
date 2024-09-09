#include "descriptors.hpp"
#include "swapchain.hpp"
#include "vk_mem_alloc.h"
#include "vma.hpp"

// std
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_handles.hpp>
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
  std::array<vk::WriteDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> writes{};
  std::array<vk::DescriptorBufferInfo, writes.size()> infos{};
  for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
    infos[i] = vk::DescriptorBufferInfo{buffers[i], 0, VK_WHOLE_SIZE};
    writes[i] = {sets[i], 0,         0, 1, vk::DescriptorType::eUniformBuffer,
                 nullptr, &infos[i]};
  }
  vlkn->getDevice().updateDescriptorSets(writes, nullptr);
}

void DescriptorSet::createPool(uint32_t maxSets) {
  vk::DescriptorPoolCreateInfo createInfo{
      {}, maxSets, poolSize};
  pool = vlkn->getDevice().createDescriptorPool(createInfo);
}

void DescriptorSet::addBinding(uint32_t bindingNo, vk::DescriptorType type, vk::ShaderStageFlags stage,
                uint32_t count) {
  vk::DescriptorSetLayoutBinding binding{bindingNo, type, count,stage};
  assert(bindings.count(bindingNo) == 0 && "Binding already taken!");
  bindings[bindingNo] = binding;
}

void DescriptorSet::createSets(const uint32_t numSets) {
  std::vector<vk::DescriptorSetLayoutBinding> layoutB;
  for (auto &b : bindings) {
    layoutB.push_back(b.second);
  }

  layout = vlkn->getDevice().createDescriptorSetLayout(
      vk::DescriptorSetLayoutCreateInfo{{}, layoutB});
  std::vector<vk::DescriptorSetLayout> layouts{};
  for (int i = 0; i < numSets; ++i) {
  layouts.push_back(layout);
  }

  vk::DescriptorSetAllocateInfo info{pool, layouts};
  sets = vlkn->getDevice().allocateDescriptorSets(info);

}

void RenderDescriptors::createSets(uint32_t numSets) {
  DescriptorSet::createSets();
  vk::BufferCreateInfo createInfo{
      {}, sizeof(UniformBuffer), vk::BufferUsageFlagBits::eUniformBuffer};
  allocs.resize(numSets);
  allocInfos.resize(numSets);
  VmaAllocationCreateInfo allocCreateInfo{
      VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, 0};
  for (size_t i = 0; i < numSets; ++i) {
    buffers.push_back(vlkn->getVma()->createBuffer(
        allocs[i], allocInfos[i], createInfo, allocCreateInfo));
  }
}

void RenderDescriptors::update(const glm::mat4 &mat, size_t idx) {
  // copy to buffer
  memcpy(allocInfos.at(idx).pMappedData, &mat, sizeof(glm::mat4));
  vk::MappedMemoryRange range(allocInfos.at(idx).deviceMemory,0,VK_WHOLE_SIZE);
  vlkn->getDevice().flushMappedMemoryRanges(range);
}

void TraceDescriptors::writeSetup(vk::AccelerationStructureKHR &pTLAS) {
  vk::WriteDescriptorSetAccelerationStructureKHR
      descriptorAccelerationStructure{pTLAS};
  vk::WriteDescriptorSet tlasWrite{
  sets.front(), 0, 0, 1, vk::DescriptorType::eAccelerationStructureKHR};
  tlasWrite.pNext = &descriptorAccelerationStructure;
  write = tlasWrite;
  vlkn->getDevice().updateDescriptorSets(write, nullptr);
}

} // namespace rn
