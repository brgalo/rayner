#include "glm/glm.hpp"
#include <array>

namespace rn {

class Colormap {
  Colormap();
  std::array<glm::vec3, 256> colors;
};
} // namespace rn
