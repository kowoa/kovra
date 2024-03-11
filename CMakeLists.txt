cmake_minimum_required(VERSION 3.28)
project(kovra VERSION 0.1.0)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)
add_subdirectory(external)

# Compile shaders --------------------------------------------------------------
find_program(
  GLSL_VALIDATOR
  NAMES glslangValidator
  HINTS /usr/bin /usr/local/bin)
file(GLOB_RECURSE GLSL_SOURCE_FILES "${PROJECT_SOURCE_DIR}/shaders/*.frag"
     "${PROJECT_SOURCE_DIR}/shaders/*.vert"
     "${PROJECT_SOURCE_DIR}/shaders/*.comp")
# Compile each GLSL shader to SPIR-V
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "Compiling shader: ${GLSL}")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/shaderbuild")
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaderbuild/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    # SPIR-V 1.4 is required for Vulkan ray tracing
    COMMAND ${GLSL_VALIDATOR} -V --target-env spirv1.4 ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()
# Before building the project, make sure the shaders are compiled
add_custom_target(SHADERS DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${PROJECT_NAME} SHADERS)
# -------------------------------------------------------------------------------
