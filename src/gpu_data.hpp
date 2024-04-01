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
    VkDeviceAddress vertex_buffer;
    glm::mat4x4 object_transform = glm::identity<glm::mat4x4>();
};
} // namespace kovra
