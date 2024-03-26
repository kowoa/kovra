#pragma once

#include "gpu_data.hpp"
#include <vulkan/vulkan.hpp>

namespace kovra {
struct VertexInputDescription {
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attributes;
    vk::PipelineVertexInputStateCreateFlags flags;
};

struct Vertex {
  public:
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;

    [[nodiscard]] static VertexInputDescription get_vertex_desc() {
        VertexInputDescription desc{};
        desc.bindings = {vk::VertexInputBindingDescription(
            0, sizeof(Vertex), vk::VertexInputRate::eVertex)};
        desc.attributes = {
            vk::VertexInputAttributeDescription(
                0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
            vk::VertexInputAttributeDescription(
                1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
            vk::VertexInputAttributeDescription(
                2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription(
                3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))};
        return desc;
    }

    [[nodiscard]] GpuVertexData as_gpu_data() const {
        return GpuVertexData{
            .position = position,
            .uv_x = uv.x,
            .normal = normal,
            .uv_y = uv.y,
            .color = glm::vec4(color, 1.0f)};
    }
};
} // namespace kovra
