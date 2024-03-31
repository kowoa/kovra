#include "render_resources.hpp"
#include "asset-loader.hpp"
#include "device.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"

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
RenderResources::add_mesh_asset(std::unique_ptr<MeshAsset> &&mesh_asset)
{
    mesh_assets.push_back(std::move(mesh_asset));
}
void
RenderResources::add_texture(
  const std::string &&name,
  std::unique_ptr<GpuImage> &&texture
)
{
    textures.emplace(std::move(name), std::move(texture));
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

} // namespace kovra
