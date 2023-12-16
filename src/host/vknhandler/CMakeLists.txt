include_directories(../renderer)

add_library(vknhandler vknhandler.cpp
                       vknhandler.hpp
                       vma.cpp
                       vma.hpp)
      
target_link_libraries(vknhandler PRIVATE Vulkan::Vulkan
                                 PRIVATE GPUOpen::VulkanMemoryAllocator
                                 PRIVATE renderer)