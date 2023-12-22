add_library(geometry geometry.cpp
                     geometry.hpp)

include_directories(../../libs/Vulkan-Hpp/glfw/include
                    ../vknhandler
                    ../../../libs/VulkanMemoryAllocator/include)