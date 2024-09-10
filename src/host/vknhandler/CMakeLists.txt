include_directories(../renderer)

add_library(vknhandler vknhandler.cpp
                       vknhandler.hpp
                       vma.cpp
                       vma.hpp
                       vmaimpl.cpp)
      
target_link_libraries(vknhandler PRIVATE Vulkan::Vulkan
                                 PUBLIC VulkanMemoryAllocator
                                 PRIVATE renderer)