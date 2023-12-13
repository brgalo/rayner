add_library(vknhandler vknhandler.cpp
                        vknhandler.hpp)
target_link_libraries(vknhandler PRIVATE Vulkan::Vulkan)