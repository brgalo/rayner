add_library(vknhandler vknhandler.cpp
                        vknhandler.hpp)
add_library(window window.cpp 
                   window.hpp)
target_link_libraries(window PRIVATE Vulkan::Vulkan
                             PRIVATE glfw)
target_link_libraries(vknhandler PRIVATE Vulkan::Vulkan
                                 PRIVATE window)