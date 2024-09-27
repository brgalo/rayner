#pragma once

#include "imgui.h"
#include "swapchain.hpp"
#include "vknhandler.hpp"
#include "window.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "state.hpp"

namespace rn {

class Gui {
public:
  Gui(VulkanHandler &vlkn, Window &window, const SwapChain &swapchain, std::shared_ptr<std::vector<std::string>> triangleNames);
  ~Gui();
  void recreateFramebuffers(const SwapChain &swapchain);
  void render(vk::CommandBuffer &buffer, uint32_t idx, vk::Extent2D extent);
  std::shared_ptr<State> state = std::make_shared<State>();
  std::shared_ptr<std::vector<std::string>> triangleNames;

private:
  VulkanHandler &vlkn;
  Window &window;
  void createDescriptorPool();
  void createContext(const SwapChain &swapchain);
  void uploadFonts();
  void createFramebuffers(const SwapChain &swapchain);
  void createRenderPass(vk::Format format);
  void destroyRenderPass();
  void destroyFramebuffers();
  vk::DescriptorPool pool;
  std::vector<vk::Framebuffer> framebuffers;
  vk::RenderPass renderPass;

  void oriMenu();

  // gui
  void gui();
};
}