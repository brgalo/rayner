add_library(raytracer raytracer.hpp
                      raytracer.cpp)

target_link_libraries(raytracer vknhandler
                                Vulkan::Vulkan
                                pipeline)