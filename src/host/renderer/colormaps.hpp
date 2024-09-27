#include "glm/glm.hpp"
#include "vma.hpp"
#include <array>

namespace rn {

class Colormaps {
  public:
    Colormaps(std::shared_ptr<VulkanHandler> vulkn_);
  const std::array<glm::vec3, 256> &getViridisColors() const { return viridisColors; };
  private:
    const std::array<glm::vec3, 256> viridisColors;
    vk::Image viridisImage;
    void upload();
    std::shared_ptr<VulkanHandler> vlkn;
};

} // namespace rn
