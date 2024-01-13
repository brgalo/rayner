#pragma once
#include "keyboard.hpp"
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

using namespace glm;

namespace rn {
class Camera {
public:
  Camera(Window& window);

  void setViewDirection(glm::vec3 position, glm::vec3 direction,
                        glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
  void setViewTarget(glm::vec3 position, glm::vec3 target,
                     glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
  void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
  void setOrtographic();
  void setPerspective();
  bool updateView(float frameTime);
  mat4 &getProjView() { return projectionView; };

private:
  void setOrtographic(float left, float right, float top, float bottom,
                      float near, float far);
  void setPerspective(float fovy, float aspect, float near,
                                      float far);
  void calcViewProj();
  Window &window;
  mat4 projectionView{1.f};
  mat4 projection{1.f};
  mat4 view{1.f};

  KeyboardController controller;
};
}