#include "../vknhandler/vknhandler.hpp"
#include <unordered_map>

namespace rn {


class DescriptorSetLayout {
public:
  class Builder {
  public:
    Builder(const VulkanHandler &vlkn) : device{vlkn.getDevice()} {}

    Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType,
                        vk::ShaderStageFlags stageFlags, uint32_t count = 1);
    std::unique_ptr<DescriptorSetLayout> build() const;

  private:
    const vk::Device &device;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
  };

  DescriptorSetLayout(
      const vk::Device &device,
      std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayout();
  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  vk::DescriptorSetLayout getDescriptorSetLayout() const {
    return setLayout;
  }
  const vk::DescriptorSetLayout *getDescriptorSetLayoutPointer() const {
    return &setLayout;
  }

private:
  const vk::Device &device;
  vk::DescriptorSetLayout setLayout;
  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

  std::vector<vk::DescriptorSet> sets;

  friend class DescriptorWriter;
};

class DescriptorPool {
public:
  class Builder {
  public:
    Builder(const vk::Device &device) : device{device} {}

    Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<DescriptorPool> build() const;
    

  private:
    const vk::Device &device;
    std::vector<vk::DescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    vk::DescriptorPoolCreateFlags poolFlags{};
  };

  DescriptorPool(const vk::Device &device, uint32_t maxSets,
                 vk::DescriptorPoolCreateFlags poolFlags,
                 const std::vector<vk::DescriptorPoolSize> &poolSizes);
  ~DescriptorPool();
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

std::vector<vk::DescriptorSet>
  allocateDescriptor(const vk::DescriptorSetLayout descriptorSetLayout) const;

  void freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const;

  void resetPool();
  
  vk::DescriptorPool &getDescriptorPool() {return descriptorPool;};
private:
  const vk::Device &device;
  vk::DescriptorPool descriptorPool;

  friend class DescriptorWriter;
};

class DescriptorWriter {
public:
  DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

  bool writeandBuildTLAS(uint32_t binding, vk::AccelerationStructureKHR *pTLAS,
                         vk::DescriptorSet &descriptorSet,
                         vk::DescriptorBufferInfo *bufferInfo);

  DescriptorWriter &writeBuffer(uint32_t binding,
                                vk::DescriptorBufferInfo *bufferInfo);
  DescriptorWriter &writeImage(uint32_t binding,
                               vk::DescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

private:
  DescriptorSetLayout &setLayout;
  DescriptorPool &pool;
  std::vector<vk::WriteDescriptorSet> writes;
};


} // namespace rn










































namespace rn {


// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
    uint32_t binding, vk::DescriptorType descriptorType,
    vk::ShaderStageFlags stageFlags, uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  vk::DescriptorSetLayoutBinding layoutBinding{binding, descriptorType, count,
                                               stageFlags};
  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<DescriptorSetLayout>
DescriptorSetLayout::Builder::build() const {
  return std::make_unique<DescriptorSetLayout>(device, bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(
    const vk::Device &device,
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : device{device}, bindings{bindings} {
  std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{{},
                                                            setLayoutBindings};

  setLayout = device.createDescriptorSetLayout(descriptorSetLayoutInfo);
}

DescriptorSetLayout::~DescriptorSetLayout() {
  device.destroyDescriptorSetLayout(setLayout);
}

// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder &
DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType,
                                     uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

DescriptorPool::Builder &
DescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
  return std::make_unique<DescriptorPool>(device, maxSets, poolFlags,
                                          poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(
    const vk::Device &device, uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize> &poolSizes)
    : device{device} {
  vk::DescriptorPoolCreateInfo descriptorPoolInfo{{}, maxSets, poolSizes};

  descriptorPool = device.createDescriptorPool(descriptorPoolInfo);
}

DescriptorPool::~DescriptorPool() {
  device.destroyDescriptorPool(descriptorPool);
}

// more accuarately allocates descriptorSET not a single one
std::vector<vk::DescriptorSet> DescriptorPool::allocateDescriptor(
    const vk::DescriptorSetLayout descriptorSetLayout) const {
  vk::DescriptorSetAllocateInfo allocInfo{descriptorPool, descriptorSetLayout};

  // Might want to create a "DescriptorPoolManager" class that handles this
  // case, and builds a new pool whenever an old pool fills up. But this is
  // beyond our current scope
  return device.allocateDescriptorSets(allocInfo);
}

void DescriptorPool::freeDescriptors(
    std::vector<vk::DescriptorSet> &descriptors) const {
  device.freeDescriptorSets(descriptorPool, descriptors);
}

void DescriptorPool::resetPool() {
  device.resetDescriptorPool(descriptorPool);
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout,
                                   DescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}


// raytracing


bool DescriptorWriter::writeandBuildTLAS(uint32_t binding,
                                         vk::AccelerationStructureKHR *pTLAS,
                                         vk::DescriptorSet &descriptorSet,
                                         vk::DescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain TLAS binding!");
  VkWriteDescriptorSetAccelerationStructureKHR
      descriptorAccelerationStructure{};
  descriptorAccelerationStructure.sType =
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
  descriptorAccelerationStructure.accelerationStructureCount = 1;
  descriptorAccelerationStructure.pAccelerationStructures = pTLAS;

  VkWriteDescriptorSet tlasWrite{};
  tlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  tlasWrite.pNext = &descriptorAccelerationStructure;
  tlasWrite.dstSet = descriptorSet;
  tlasWrite.descriptorCount = 1;
  tlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

  writes.push_back(tlasWrite);

  VkWriteDescriptorSet outputWrite{};
  outputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  outputWrite.dstSet = 0;
  outputWrite.dstBinding = 1;
  outputWrite.descriptorCount = 1;
  outputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  outputWrite.pBufferInfo = bufferInfo;
  writes.push_back(outputWrite);

  return this->build(descriptorSet);
}


DescriptorWriter &
DescriptorWriter::writeBuffer(uint32_t binding,
                              vk::DescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write{,};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  writes.push_back(write);
  return *this;
}

DescriptorWriter &
DescriptorWriter::writeImage(uint32_t binding,
                             vk::DescriptorImageInfo *imageInfo) {
  assert(setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  writes.push_back(write);
  return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
  bool success =
      pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0,
                         nullptr);
}

}