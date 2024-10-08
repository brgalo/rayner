#pragma once
#include "swapchain.hpp"
#include "vknhandler.hpp"

// std
#include <cstddef>
#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_structs.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace rn {

struct RenderPushConstsData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

struct RtPushConstsData {};

class DescriptorSet {
public:
  DescriptorSet(std::shared_ptr<VulkanHandler> &vlkn_) : vlkn(vlkn_){};
  virtual ~DescriptorSet();

  DescriptorSet(const DescriptorSet &) = delete;

  std::vector<vk::DescriptorSet> getSets() { return sets; };
  virtual void updateSets(); 
  void addPoolSize(vk::DescriptorType type, uint32_t count) {
    poolSize.push_back({type, count});
  };
  void addBinding(uint32_t binding, vk::DescriptorType type,
                  vk::ShaderStageFlags, uint32_t count = 1);

  vk::DescriptorSetLayout &getLayout() { return layout; };

  vk::CommandBuffer bind();

protected:
  virtual void createSets(uint32_t numSets = SwapChain::MAX_FRAMES_IN_FLIGHT);
  std::shared_ptr<VulkanHandler> vlkn = nullptr;
  std::vector<vk::Buffer> buffers;
  std::vector<VmaAllocation> allocs;
  std::vector<VmaAllocationInfo> allocInfos;
  void createPool(uint32_t maxSets);
  void freeSets();
  vk::DescriptorPool pool;
  vk::DescriptorSetLayout layout;


  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;
  std::vector<vk::DescriptorSet> sets;
private:


  std::vector<vk::DescriptorPoolSize> poolSize;
};

class RenderDescriptors : public DescriptorSet {
  struct UniformBuffer;
public:
  RenderDescriptors(std::shared_ptr<VulkanHandler> vlkn) : DescriptorSet(vlkn) {
    addPoolSize(vk::DescriptorType::eUniformBuffer, 2);
    createPool(SwapChain::MAX_FRAMES_IN_FLIGHT);

    addBinding(0, vk::DescriptorType::eUniformBuffer,
               vk::ShaderStageFlagBits::eVertex);
    RenderDescriptors::createSets();
    ub.mat = ub.projectionViewMatrix(vk::Extent2D{1000, 500});
    updateSets();
  };
  void update(const glm::mat4 &val, size_t idx);

private:
  struct UniformBuffer {
    glm::mat4 mat;
    glm::mat4 projectionViewMatrix(vk::Extent2D const & extent )
    {
      float fov = glm::radians( 45.0f );
      if ( extent.width > extent.height )
      {
        fov *= static_cast<float>( extent.height ) / static_cast<float>( extent.width );
      }

      glm::mat4x4 model      = glm::mat4x4( 1.0f );
      glm::mat4x4 view       = glm::lookAt( glm::vec3( -5.0f, 3.0f, -10.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) );
      glm::mat4x4 projection = glm::perspective( fov, 1.0f, 0.1f, 100.0f );
      // clang-format off
      glm::mat4x4 clip = glm::mat4x4( 1.0f,  0.0f, 0.0f, 0.0f,
                                      0.0f, -1.0f, 0.0f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 1.0f );  // vulkan clip space has inverted y and half z !
      // clang-format on 
      return clip * projection * view * model;}
  } ub;

  void createSets(uint32_t numSets = SwapChain::MAX_FRAMES_IN_FLIGHT) override;
};

class TraceDescriptors : public DescriptorSet {
public:
  TraceDescriptors(std::shared_ptr<VulkanHandler> vlkn_) : DescriptorSet(vlkn_) {
    addPoolSize(vk::DescriptorType::eAccelerationStructureKHR, 1);
    createPool(1);

    addBinding(0, vk::DescriptorType::eAccelerationStructureKHR,
               vk::ShaderStageFlagBits::eRaygenKHR);


    layout = vlkn->getDevice().createDescriptorSetLayout(
      vk::DescriptorSetLayoutCreateInfo{{},bindings.at(0)});
      vk::DescriptorSetAllocateInfo allocInfo{pool, layout};
  sets = vlkn->getDevice().allocateDescriptorSets(allocInfo);
  };

  void  writeSetup(vk::AccelerationStructureKHR &pTLAS);
private:
vk::WriteDescriptorSet write;
};

class ComputeDescriptors : public DescriptorSet {
public:
  ComputeDescriptors(std::shared_ptr<VulkanHandler> vlkn_) : DescriptorSet(vlkn_) {
    createPool(0);
  };
};

} // namespace rn