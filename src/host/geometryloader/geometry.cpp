#include "geometry.hpp"

namespace rn {
void GeometryHandler::uploadVertexData() {
  vertex = vma->uploadGeometry(reinterpret_cast<const void *>(&coloredCubeData),
                      sizeof(VertexPC), alloc);
}

}