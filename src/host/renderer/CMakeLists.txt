add_library(renderer renderer.cpp
                     renderer.hpp
                     window.cpp 
                     window.hpp
                     swapchain.cpp
                     swapchain.hpp
                     camera.hpp
                     camera.cpp
                     keyboard.hpp
                     keyboard.cpp)

add_library(pipeline pipeline.cpp
                     pipeline.hpp
                     descriptors.hpp
                     descriptors.cpp)

add_library(gui      gui.cpp
                     gui.hpp
                     ${IMGUI_INCLUDES}
                     ${IMPLOT_INCLUDES}
                     )

include_directories(../../libs/Vulkan-Hpp/glfw/include
                    ../vknhandler
                    ../../../libs/VulkanMemoryAllocator/include
                    .)
target_link_libraries(renderer PRIVATE Vulkan::Vulkan
                                       glfw
                                       pipeline
                                       gui
                                       vknhandler
                                       PUBLIC GPUOpen::VulkanMemoryAllocator
                                       )