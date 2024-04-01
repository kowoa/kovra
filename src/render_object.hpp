#pragma once

#include "descriptor.hpp"

#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Material;
class DrawContext;
class GpuImage;
class Renderer;

struct MaterialInstance
{
    const Material &material;
    const vk::DescriptorSet desc_set;
};

struct RenderObject
{
    const uint32_t index_count;
    const uint32_t first_index;
    const vk::Buffer index_buffer;

    const MaterialInstance &material;
};

// Base class for a renderable dynamic object
class IRenderable
{
    virtual void draw(const glm::mat4 &parent, DrawContext &ctx) = 0;
};

struct GltfMetallicRoughness
{
    std::unique_ptr<Material> opaque_material;
    std::unique_ptr<Material> transparent_material;
    vk::DescriptorSetLayout material_layout;

    struct MaterialConstants
    {
        const glm::vec4 color_factors;
        const glm::vec4 metal_rough_factors;
        // Padding for uniform buffers
        const glm::vec4 padding[14];
    };

    struct MaterialResources
    {
        const std::unique_ptr<GpuImage> color_image;
        const std::unique_ptr<GpuImage> metal_rough_image;
        const vk::Buffer data_buffer;
        const uint32_t data_buffer_offset;
    };

    DescriptorWriter writer;

    void build_materials(const Renderer &renderer);
    // Destroy everything that was created in build_materials
    void clear_resources(const vk::Device &device);
    // Create a descriptor set and return a fully built MaterialInstance
    MaterialInstance write_material(
      const vk::Device &device,
      const MaterialResources &resources,
      DescriptorAllocator &desc_allocator
    );
};
}
