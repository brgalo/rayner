#include "geometry.hpp"
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace rn {

std::vector<vk::VertexInputAttributeDescription> GeometryHandler::getAttributeDescription() {
  std::vector<vk::VertexInputAttributeDescription> attr{};
  attr.push_back(
      {0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPC, pos)});
  attr.push_back(
      {1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(VertexPC, col)});
  return attr;
}

std::vector<vk::VertexInputBindingDescription>
GeometryHandler::getInputDescription() {
  return {{0, sizeof(VertexPC), vk::VertexInputRate::eVertex}};
}

void GeometryHandler::uploadVertexData() {
  vertex = vma->uploadGeometry(reinterpret_cast<const void *>(&coloredCubeData),
                      sizeof(VertexPC)*36, alloc);
}

}