cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project("RAYNER")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

find_package(Vulkan REQUIRED COMPONENTS glslc)

#glfw
set(GLFW_DIR "${PROJECT_SOURCE_DIR}/libs/Vulkan-Hpp/glfw")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL "Generate installation target" OFF)
set(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(libs/Vulkan-Hpp/glfw)
include_directories(libs/Vulkan-Hpp/glfw/include)

#glm
add_subdirectory(libs/Vulkan-Hpp/glm)

add_subdirectory(src/host)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE   vknhandler
                                    renderer
                                    Vulkan::Vulkan)

include_directories(src/host)