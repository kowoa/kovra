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

struct PbrMaterialInstanceCreateInfo
{
    const GpuImage &albedo_texture;
    const vk::Sampler &albedo_sampler;
    const GpuImage &metal_rough_texture;
    const vk::Sampler &metal_rough_sampler;
    const vk::Buffer &material_buffer; // Buffer containing GpuPbrMaterialData
    const uint32_t material_buffer_offset;
    const MaterialPass pass;
};

class PbrMaterial
{
  public:
    explicit PbrMaterial(
      const vk::Device &device,
      const vk::DescriptorSetLayout &scene_desc_layout,
      const vk::Format &color_attachment_format,
      const vk::Format &depth_attachment_format,
      const vk::SampleCountFlagBits &sample_count
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
    ) const;

  private:
    std::shared_ptr<Material> opaque_material;
    std::shared_ptr<Material> transparent_material;
    vk::UniqueDescriptorSetLayout material_layout;
    std::unique_ptr<DescriptorWriter> desc_writer;
};
}
