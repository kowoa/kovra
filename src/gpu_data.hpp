#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

namespace kovra {
struct GpuVertexData
{
    glm::vec3 position;
    glm::f32 uv_x;
    glm::vec3 normal;
    glm::f32 uv_y;
    glm::vec4 color;
};

struct GpuCameraData
{
    glm::mat4x4 viewproj;
    glm::f32 near;
    glm::f32 far;
};

struct GpuSceneData
{
    GpuCameraData camera;
    glm::vec4 ambient_color;
    glm::vec4 sunlight_direction;
    glm::vec4 sunlight_color;
};

struct GpuPushConstants
{
    const glm::mat4x4 object_transform = glm::identity<glm::mat4x4>();
    const VkDeviceAddress vertex_buffer;
    // Padding
    const std::byte padding[55];
};

struct GpuPbrMaterialData
{
    const glm::vec4 color_factors;
    const glm::vec4 metal_rough_factors;
    // Padding for uniform buffers
    const glm::vec4 padding[14];
};
} // namespace kovra
