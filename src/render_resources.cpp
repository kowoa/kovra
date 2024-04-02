#include "render_resources.hpp"
#include "asset_loader.hpp"
#include "device.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "pbr_material.hpp"
#include "render_object.hpp"

namespace kovra {
RenderResources::RenderResources(std::shared_ptr<Device> device)
  : device{ std::move(device) }
{
}
RenderResources::~RenderResources()
{
    mesh_assets.clear();

    for (const auto &[_, desc_set_layout] : desc_set_layouts) {
        device->get().destroyDescriptorSetLayout(desc_set_layout);
    }
    desc_set_layouts.clear();

    for (const auto &[_, sampler] : samplers) {
        device->get().destroySampler(sampler);
    }
    samplers.clear();

    materials.clear();
}

void
RenderResources::add_material(const std::string &&name, Material &&material)
{
    materials.emplace(
      std::move(name), std::make_shared<Material>(std::move(material))
    );
}
void
RenderResources::add_sampler(
  const vk::Filter &&filter,
  const vk::Sampler &&sampler
)
{
    samplers.emplace(std::move(filter), std::move(sampler));
}
void
RenderResources::add_desc_set_layout(
  const std::string &&name,
  const vk::DescriptorSetLayout &&desc_set_layout
)
{
    desc_set_layouts.emplace(std::move(name), std::move(desc_set_layout));
}
void
RenderResources::add_mesh_asset(MeshAsset &&mesh_asset)
{
    auto mesh_asset_owned = std::make_shared<MeshAsset>(std::move(mesh_asset));

    // Update loaded_nodes
    {
        auto new_node = std::make_shared<MeshNode>();
        new_node->mesh_asset = mesh_asset_owned;
        new_node->local_transform = glm::mat4{ 1.0f };
        new_node->world_transform = glm::mat4{ 1.0f };

        for (auto &surface : new_node->mesh_asset->surfaces) {
            surface.material_instance = default_material_instance;
        }

        loaded_nodes.emplace(mesh_asset_owned->name, std::move(new_node));
    }

    // Update mesh_assets
    mesh_assets.push_back(mesh_asset_owned);
}
void
RenderResources::add_texture(
  const std::string &&name,
  std::unique_ptr<GpuImage> &&texture
)
{
    textures.emplace(std::move(name), std::move(texture));
}
void
RenderResources::set_pbr_material(std::unique_ptr<PbrMaterial> &&material)
{
    pbr_material = std::move(material);
}

[[nodiscard]] const Material &
RenderResources::get_material(const std::string &name) const
{
    if (materials.find(name) == materials.end()) {
        throw std::runtime_error("Material not found: " + name);
    }
    return *materials.at(name);
}
[[nodiscard]] std::shared_ptr<Material>
RenderResources::get_material_owned(const std::string &name) const
{
    if (materials.find(name) == materials.end()) {
        throw std::runtime_error("Material not found: " + name);
    }
    return materials.at(name);
}
[[nodiscard]] const vk::Sampler &
RenderResources::get_sampler(vk::Filter filter) const
{
    if (samplers.find(filter) == samplers.end()) {
        throw std::runtime_error("Sampler not found");
    }
    return samplers.at(filter);
}
[[nodiscard]] const vk::DescriptorSetLayout &
RenderResources::get_desc_set_layout(const std::string &name) const
{
    if (desc_set_layouts.find(name) == desc_set_layouts.end()) {
        throw std::runtime_error("Descriptor set layout not found: " + name);
    }
    return desc_set_layouts.at(name);
}
[[nodiscard]] const GpuImage &
RenderResources::get_texture(const std::string &name) const
{
    if (textures.find(name) == textures.end()) {
        throw std::runtime_error("Texture not found: " + name);
    }
    return *textures.at(name);
}
[[nodiscard]] const PbrMaterial &
RenderResources::get_pbr_material() const
{
    if (!pbr_material) {
        throw std::runtime_error("PBR material not found");
    }
    return *pbr_material;
}
[[nodiscard]] const MeshNode &
RenderResources::get_mesh_node(const std::string &name) const
{
    if (loaded_nodes.find(name) == loaded_nodes.end()) {
        throw std::runtime_error("Mesh node not found: " + name);
    }
    return *loaded_nodes.at(name);
}

} // namespace kovra
