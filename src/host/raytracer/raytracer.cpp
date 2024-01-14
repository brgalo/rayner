#include "raytracer.hpp"
#include "vknhandler.hpp"

namespace rn {
Raytracer::Raytracer(std::shared_ptr<VulkanHandler> vlkn_) : vlkn(vlkn_) {}

Raytracer::~Raytracer() {
  
}

}