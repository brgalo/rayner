#include "geometry.hpp"
#include <glm/fwd.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


// fancy stuff to reduce to unique vertices!
#include <functional>

namespace rn {
template <typename T, typename... Rest>
void hashCombine(std::size_t &seed, const T &v, const Rest &...rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
};

} // namespace oray

namespace std {
template <> struct hash<glm::vec3> {
  size_t operator()(glm::vec3 const &vertex) const {
    size_t seed = 0;
    rn::hashCombine(seed, vertex.x, vertex.y, vertex.z);
    return seed;
  }
};
} // namespace std

namespace rn {

GeometryHandler::GeometryHandler(std::shared_ptr<VMA> vma_) : vma(vma_) {
  loadObj("geom/plane.obj");
  vertex = vma->uploadVertices(vertices, vertexAlloc);
  index = vma->uploadIndices(indices, indexAlloc);  
}

std::vector<vk::VertexInputAttributeDescription> GeometryHandler::getAttributeDescription() {
  std::vector<vk::VertexInputAttributeDescription> attr{};
  attr.push_back({0, 0, vk::Format::eR32G32B32Sfloat, 0});
  return attr;
}

std::vector<vk::VertexInputBindingDescription>
GeometryHandler::getInputDescription() {
  return {{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}};
}

void GeometryHandler::uploadVertexData() {
  vertex = vma->uploadGeometry(reinterpret_cast<const void *>(&coloredCubeData),
                      sizeof(VertexPC)*36, vertexAlloc);
}

void GeometryHandler::loadObj(const std::string &filePath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        filePath.c_str())) {
    throw std::runtime_error(warn + err);
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<glm::vec3, uint32_t> uniqueVertices{};
  unsigned int nTrianglesWithCurrentMesh = 0;

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      glm::vec3 vertex{};

      if (index.vertex_index >= 0) {
        vertex.x = attrib.vertices[3 * index.vertex_index + 0];
        vertex.y = attrib.vertices[3 * index.vertex_index + 1];
        vertex.z = attrib.vertices[3 * index.vertex_index + 2];
        };

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    }

    // store number of triangles per mesh
    MeshIdx idx;
    idx.data.x = shape.mesh.num_face_vertices.size();
    nTrianglesWithCurrentMesh += idx.data.x;
    idx.data.y = nTrianglesWithCurrentMesh;
    triangleToMeshIdx.push_back(idx);
  }

}

}