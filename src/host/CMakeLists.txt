add_subdirectory(renderer)
add_subdirectory(vknhandler)
add_subdirectory(geometryloader)
add_subdirectory(raytracer)

include_directories(.)

add_library(rayner rayner.hpp
                   rayner.cpp)

                   target_link_libraries(rayner renderer
                                                vknhandler
                                                geometry
                                                )