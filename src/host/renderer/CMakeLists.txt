add_library(renderer renderer.cpp
                     renderer.hpp
                     gui.cpp
                     gui.hpp)
add_library(window window.cpp 
                   window.hpp)
add_library(swapchain swapchain.cpp
                      swapchain.hpp)
target_link_libraries(window PRIVATE Vulkan::Vulkan
                             PRIVATE glfw
                             PRIVATE swapchain)

target_link_libraries(renderer PRIVATE Vulkan::Vulkan
                               PRIVATE window
                               PRIVATE swapchain)