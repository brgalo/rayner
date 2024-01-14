#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>


#include "vk_mem_alloc.h"
#include "vma.hpp"

namespace rn {
class GeometryHandler {
public:
  GeometryHandler(std::shared_ptr<VMA> vma_);
  ~GeometryHandler() {
    vma->destroyBuffer(vertexAlloc, vertex);
    vma->destroyBuffer(indexAlloc, index);
  }
  vk::CommandBuffer bindVertexBuffer(vk::CommandBuffer commandBuffer);
  void uploadVertexData();
  static std::vector<vk::VertexInputBindingDescription> getInputDescription();
  static std::vector<vk::VertexInputAttributeDescription>
  getAttributeDescription();
  vk::Buffer getVert() { return vertex; };
  vk::Buffer getIdx() { return index; };
  void loadObj(const std::string &fName);

  std::vector<glm::vec3> vertices{};
  std::vector<uint32_t> indices{};
  std::shared_ptr<VMA> vma = nullptr;
struct VertexPC
{
  glm::vec4 pos;  // Position
  glm::vec4 col;   // Color
};
  struct MeshIdx {
    // .x = nTrianglesInCurrentMesh
    // .y = cumTrianglesWithCurrentMesh
    glm::uvec4 data{0};
  };

  std::vector<MeshIdx> triangleToMeshIdx{};

static constexpr VertexPC coloredCubeData[] = {
    // red face
    {{-1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{-1.0f,  1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{ 1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{ 1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{-1.0f,  1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{ 1.0f,  1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    // green face
    {{-1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{ 1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{-1.0f,  1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{-1.0f,  1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{ 1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{ 1.0f,  1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    // blue face
    {{-1.0f,  1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f, -1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f,  1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f,  1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f, -1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    // yellow face
    {{1.0f,  1.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
    {{1.0f,  1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
    {{1.0f, -1.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
    {{1.0f, -1.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
    {{1.0f,  1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
    {{1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
    // magenta face
    {{ 1.0f, 1.0f,  1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f,  1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    {{ 1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    {{ 1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f,  1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    // cyan face
    {{ 1.0f, -1.0f,  1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
    {{ 1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
    {{-1.0f, -1.0f,  1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
    {{-1.0f, -1.0f,  1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
    {{ 1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
    {{-1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
};

private:
  vk::Buffer vertex;
  vk::Buffer index;
  VmaAllocation vertexAlloc;
  VmaAllocation indexAlloc;
};
}