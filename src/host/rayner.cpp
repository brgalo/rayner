#include "rayner.hpp"
#include <chrono>

namespace rn {

void Rayner::run() {
  

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (renderer.run()) {
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();
    currentTime = newTime;

    renderer.updateCamera(frameTime);
    
    renderer.render(geom.getVert());
  }
}

}