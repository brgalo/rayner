#pragma once

#include <cstdint>
namespace rn {
struct State {

  static bool itemGetter(void *data, int idx, const char **out_str);

  // point launches
  uint64_t currTri = 0;
  uint64_t nPoints = 0;

  // ray launches
  uint64_t nRays = 0;


  bool pLaunch = false;
  bool rLaunch = false;
};

};