#pragma once

#include "glm/glm.hpp"

namespace kovra {
struct GpuVertexData {
    glm::vec3 position;
    glm::f32 uv_x;
    glm::vec3 normal;
    glm::f32 uv_y;
    glm::vec4 color;
};

struct GpuCameraData {
    glm::mat4x4 viewproj;
    glm::f32 near;
    glm::f32 far;
};

struct GpuSceneData {
    GpuCameraData camera;
    glm::vec4 ambient_color;
    glm::vec4 sunlight_direction;
    glm::vec4 sunlight_color;
};
} // namespace kovra
