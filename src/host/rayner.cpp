#include "rayner.hpp"

namespace rn {

void Rayner::run() {
  while (renderer.run()) {
    renderer.render(geom.getVert());
  }
}

}