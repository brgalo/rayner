#pragma once

#include "vknhandler.hpp"

namespace rn {
class Raytracer {
public:
  Raytracer(std::shared_ptr<VulkanHandler> vlkn_);
  ~Raytracer();

private:
  std::shared_ptr<VulkanHandler> vlkn;
  void buildBlas();
  void buildTlas();
  
};

}