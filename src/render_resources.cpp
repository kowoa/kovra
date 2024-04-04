#include "render_resources.hpp"
#include "asset_loader.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "pbr_material.hpp"
#include "render_object.hpp"

namespace kovra {
RenderResources::RenderResources(std::shared_ptr<Device> device)
  : device{ device }
  , material_buffer{ std::make_unique<GpuBuffer>(
      device->get_allocator_owned(),
      sizeof(GpuPbrMaterialData),
      vk::BufferUsageFlagBits::eUniformBuffer,
      VMA_MEMORY_USAGE_CPU_TO_GPU,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
{
    auto default_material_data =
      GpuPbrMaterialData{ .color_factors = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                          .metal_rough_factors =
                            glm::vec4(1.0f, 0.5f, 0.0f, 0.0f),
                          ._padding = {} };
    material_buffer->write(&default_material_data, sizeof(GpuPbrMaterialData));
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
        auto new_node = std::make_shared<MeshNode>(mesh_asset_owned);

        if (!default_material_instance) {
            spdlog::error("Default material instance not found");
            throw std::runtime_error("Default material instance not found");
        }

        for (auto &surface : new_node->get_mesh_asset_mut().surfaces) {
            surface.material_instance = default_material_instance;
        }

        renderables.emplace(mesh_asset_owned->name, std::move(new_node));
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
RenderResources::set_pbr_material(
  std::unique_ptr<PbrMaterial> &&material,
  const Device &device,
  DescriptorAllocator &global_desc_allocator
)
{
    pbr_material = std::move(material);
    default_material_instance =
      std::make_shared<MaterialInstance>(pbr_material->create_material_instance(
        { .albedo_texture = get_texture("white"),
          .albedo_sampler = get_sampler(vk::Filter::eLinear),
          .metal_rough_texture = get_texture("white"),
          .metal_rough_sampler = get_sampler(vk::Filter::eLinear),
          .material_buffer = material_buffer->get(),
          .material_buffer_offset = 0,
          .pass = MaterialPass::Opaque },
        device,
        global_desc_allocator
      ));
}
void
RenderResources::add_scene(
  const std::string &name,
  std::shared_ptr<LoadedGltfScene> &&scene
)
{
    renderables.emplace(std::move(name), std::move(scene));
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
[[nodiscard]] const IRenderable &
RenderResources::get_renderable(const std::string &name) const
{
    if (renderables.find(name) == renderables.end()) {
        throw std::runtime_error("Renderable not found: " + name);
    }
    return *renderables.at(name);
}

} // namespace kovra
