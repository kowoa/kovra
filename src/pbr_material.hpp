#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Renderer;
class GpuImage;
class Material;
class MaterialInstance;
enum class MaterialPass : uint8_t;
class DescriptorWriter;
class DescriptorAllocator;
class Device;

class PbrMaterial
{
  public:
    struct PbrMaterialInstanceCreateInfo
    {
        const GpuImage &color_image;
        const GpuImage &metal_rough_image;
        const vk::Buffer &data_buffer;
        const uint32_t data_buffer_offset;
        const MaterialPass pass;
    };

    explicit PbrMaterial(
      const vk::Device &device,
      const vk::DescriptorSetLayout &scene_desc_layout,
      const vk::Format &color_attachment_format,
      const vk::Format &depth_attachment_format
    );
    ~PbrMaterial();

    PbrMaterial() = delete;
    PbrMaterial(const PbrMaterial &) = delete;
    PbrMaterial &operator=(const PbrMaterial &) = delete;
    PbrMaterial(PbrMaterial &&) = delete;
    PbrMaterial &operator=(PbrMaterial &&) = delete;

    MaterialInstance create_material_instance(
      const PbrMaterialInstanceCreateInfo &info,
      const Device &device,
      DescriptorAllocator &desc_allocator
    );

  private:
    std::unique_ptr<Material> opaque_material;
    std::unique_ptr<Material> transparent_material;
    vk::UniqueDescriptorSetLayout material_layout;
    std::unique_ptr<DescriptorWriter> desc_writer;
};
}
