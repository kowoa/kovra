#pragma once

#include "glm/mat4x4.hpp"
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class MaterialInstance;
class DrawContext;

struct RenderObject
{
    const uint32_t index_count;
    const uint32_t first_index;
    const vk::Buffer &index_buffer;
    const MaterialInstance &material;
};

// Base class for a renderable dynamic object
class IRenderable
{
    virtual void draw(const glm::mat4 &parent, DrawContext &ctx) = 0;
};
}
