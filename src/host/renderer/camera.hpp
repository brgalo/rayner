#pragma once

#include "window.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

using namespace glm;

namespace rn {
class Camera {
public:
  Camera(const Window& window);

  const mat4 &getProjection() const { return projection; };
  const mat4 &getView() const { return view; };
  void setViewDirection(glm::vec3 position, glm::vec3 direction,
                        glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
  void setViewTarget(glm::vec3 position, glm::vec3 target,
                     glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
  void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
  void setOrtographic();

private:
  void setOrtographic(float left, float right, float top, float bottom,
                      float near, float far);
  const Window& window;
  mat4 projection{1.f};
  mat4 view{1.f};
};
}