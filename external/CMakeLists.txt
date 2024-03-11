add_library(external INTERFACE)

find_package(Vulkan REQUIRED)

add_subdirectory(SDL)
add_subdirectory(glm)
add_subdirectory(VulkanMemoryAllocator)

# imgui
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(
  imgui
  PRIVATE imgui/imgui.cpp
          imgui/imgui_demo.cpp
          imgui/imgui_draw.cpp
          imgui/imgui_widgets.cpp
          imgui/imgui_tables.cpp
          imgui/backends/imgui_impl_vulkan.cpp
          imgui/backends/imgui_impl_sdl2.cpp)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)

# tinyobjloader
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE tinyobjloader)

# spdlog
add_subdirectory(spdlog)

target_link_libraries(
  external
  INTERFACE Vulkan::Vulkan
            SDL2::SDL2
            glm
            VulkanMemoryAllocator
            imgui
            tinyobjloader
            spdlog::spdlog)
