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
    if (renderer.getGui()->state->pLaunch) {
      raytracer.traceOri(renderer.getGui()->state);
      renderer.getGui()->state->pLaunch = false;
    };
    if (renderer.getGui()->state->rLaunch) {
      raytracer.traceRays(renderer.getGui()->state);
      renderer.getGui()->state->rLaunch = false;
    };
  
    renderer.updateCamera(frameTime);
    vlkn->getGqueue().waitIdle();
    renderer.render(geom.getVert(), geom.getIdx(), geom.indices.size(),
                    raytracer.getRtConsts());
  }
}

}