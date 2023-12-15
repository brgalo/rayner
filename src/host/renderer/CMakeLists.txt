add_library(renderer renderer.cpp
                     renderer.hpp
                     gui.cpp
                     gui.hpp
                     window.cpp 
                     window.hpp
                     swapchain.cpp
                     swapchain.hpp)

include_directories(/../../libs/Vulkan-Hpp/glfw/include)
target_link_libraries(renderer PRIVATE Vulkan::Vulkan
                                       glfw)