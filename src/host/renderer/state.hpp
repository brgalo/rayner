#pragma once

#include <cstdint>
namespace rn {
struct State {
  // point launches
  uint64_t currTri = 0;
  uint64_t nPoints = 0;
  bool pLaunch = false;
};

};