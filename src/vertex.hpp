#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <vulkan/vulkan.hpp>

namespace kovra {
struct VertexInputDescription {
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attributes;
    vk::PipelineVertexInputStateCreateFlags flags;
};

struct Vertex {
  public:
    static VertexInputDescription get_vertex_desc() {
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

  private:
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
};
} // namespace kovra
