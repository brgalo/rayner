add_library(vknhandler vknhandler.cpp
                        vknhandler.hpp)
include_directories(../renderer/)
      
target_link_libraries(vknhandler PRIVATE Vulkan::Vulkan
                                 PRIVATE renderer)