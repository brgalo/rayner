#include "vma.hpp"

#define VMA_IMPLEMENTATION


#include "vk_mem_alloc.h"

#include "vma.hpp"

namespace rn {
  VMA::VMA(VulkanHandler *vkn_, VmaVulkanFunctions fun)
      : dev(vkn_->getDevice()) {
  VmaAllocatorCreateInfo info{.physicalDevice = vkn_->getPhysDevice(),
                              .device = dev,
                              .pVulkanFunctions = &fun,
                              .instance = *vkn_->getInstance()};
  vmaCreateAllocator(&info, &vma);
  };
void VMA::init() {

}
}