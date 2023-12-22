add_library(renderer renderer.cpp
                     renderer.hpp
                     gui.cpp
                     gui.hpp
                     window.cpp 
                     window.hpp
                     swapchain.cpp
                     swapchain.hpp)

add_library(pipeline pipeline.cpp
                     pipeline.hpp
                     descriptors.hpp
                     descriptors.cpp)

include_directories(../../libs/Vulkan-Hpp/glfw/include
                    ../vknhandler
                    ../../../libs/VulkanMemoryAllocator/include
                    .)
target_link_libraries(renderer PRIVATE Vulkan::Vulkan
                                       glfw
                                       pipeline
                                       )