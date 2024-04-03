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

#pragma pack(push, 1)
struct GpuSceneData
{
    // Camera
    const glm::mat4x4 viewproj;
    const glm::f32 near;
    const glm::f32 far;
    const glm::f32 _padding[2];

    // Lighting
    const glm::vec4 ambient_color;
    const glm::vec4 sunlight_direction; // w for sun power
    const glm::vec4 sunlight_color;
};
#pragma pack(pop)

struct GpuPushConstants
{
    const glm::mat4x4 object_transform = glm::identity<glm::mat4x4>();
    const VkDeviceAddress vertex_buffer;
    // Padding
    const std::byte _padding[55];
};

struct GpuPbrMaterialData
{
    const glm::vec4 color_factors;
    const glm::vec4 metal_rough_factors;
    // Padding for uniform buffers
    const glm::vec4 _padding[14];
};
} // namespace kovra
