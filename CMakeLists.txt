cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project("RAYNER")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

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

# imGui
set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/libs/imgui")
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)
set(IMGUI_INCLUDES ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                   ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
                   ${IMGUI_DIR}/imgui.cpp
                   ${IMGUI_DIR}/imgui_draw.cpp
                   ${IMGUI_DIR}/imgui_demo.cpp
                   ${IMGUI_DIR}/imgui_tables.cpp
                   ${IMGUI_DIR}/imgui_widgets.cpp)

# imPlot
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/implot)
include_directories(${IMPLOT_DIR})
set(IMPLOT_INCLUDES ${IMPLOT_DIR}/implot.h
                    ${IMPLOT_DIR}/implot_internal.h
                    ${IMPLOT_DIR}/implot.cpp   
                    ${IMPLOT_DIR}/implot_items.cpp
                    ${IMPLOT_DIR}/implot_demo.cpp)

#vulkan memory allocator
add_subdirectory(libs/VulkanMemoryAllocator)

# tiny obj loader
include_directories(libs/tinyobjloader)


include_directories(src/host/renderer/
                    src/host/vknhandler/
                    src/host/
                    src/host
                    libs/VulkanMemoryAllocator/include)

add_subdirectory(src/host)

add_executable(app main.cpp)

target_link_libraries(app PRIVATE   rayner)



add_subdirectory(src/shaders)