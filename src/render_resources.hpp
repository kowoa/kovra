#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Device;
class Material;
class MeshAsset;
class GpuImage;
class PbrMaterial;

class RenderResources
{
  public:
    explicit RenderResources(std::shared_ptr<Device> device);
    ~RenderResources();
    RenderResources() = delete;
    RenderResources(const RenderResources &) = delete;
    RenderResources &operator=(const RenderResources &) = delete;
    RenderResources(RenderResources &&) noexcept = delete;
    RenderResources &operator=(RenderResources &&) noexcept = delete;

    void add_material(const std::string &&name, Material &&material);
    void add_sampler(const vk::Filter &&filter, const vk::Sampler &&sampler);
    void add_desc_set_layout(
      const std::string &&name,
      const vk::DescriptorSetLayout &&desc_set_layout
    );
    void add_mesh_asset(std::unique_ptr<MeshAsset> &&mesh_asset);
    void
    add_texture(const std::string &&name, std::unique_ptr<GpuImage> &&texture);
    void set_pbr_material(std::unique_ptr<PbrMaterial> &&material);

    [[nodiscard]] const Material &get_material(const std::string &name) const;
    [[nodiscard]] std::shared_ptr<Material> get_material_owned(
      const std::string &name
    ) const;
    [[nodiscard]] const vk::Sampler &get_sampler(vk::Filter filter) const;
    [[nodiscard]] const vk::DescriptorSetLayout &get_desc_set_layout(
      const std::string &name
    ) const;
    [[nodiscard]] std::span<const std::unique_ptr<MeshAsset>> get_mesh_assets(
    ) const
    {
        return { mesh_assets.data(), mesh_assets.size() };
    }
    [[nodiscard]] const GpuImage &get_texture(const std::string &name) const;
    [[nodiscard]] const PbrMaterial &get_pbr_material() const;

  private:
    std::shared_ptr<Device> device;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::unordered_map<vk::Filter, vk::Sampler> samplers;
    std::unordered_map<std::string, vk::DescriptorSetLayout> desc_set_layouts;
    std::vector<std::unique_ptr<MeshAsset>> mesh_assets;
    std::unordered_map<std::string, std::unique_ptr<GpuImage>> textures;

    std::unique_ptr<PbrMaterial> pbr_material;
};
} // namespace kovra
