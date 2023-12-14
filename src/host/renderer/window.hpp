#pragma once
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <string>
#include <vulkan/vulkan_structs.hpp>

namespace rn {
class Window {
public:
  Window(int w, int h, std::string name);
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

private:

};
}