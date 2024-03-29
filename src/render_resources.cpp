#include "render_resources.hpp"
#include "device.hpp"
#include "material.hpp"
#include "mesh.hpp"

namespace kovra {
RenderResources::RenderResources(std::shared_ptr<Device> device)
  : device{ std::move(device) }
{
}
RenderResources::~RenderResources()
{
    meshes.clear();

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
RenderResources::add_mesh(
  const std::string &&name,
  std::unique_ptr<Mesh> &&mesh
)
{
    meshes.emplace(std::move(name), std::move(mesh));
}

[[nodiscard]] const Material &
RenderResources::get_material(const std::string &name) const
{
    return *materials.at(name);
}
[[nodiscard]] std::shared_ptr<Material>
RenderResources::get_material_owned(const std::string &name) const
{
    return materials.at(name);
}
[[nodiscard]] const vk::Sampler &
RenderResources::get_sampler(vk::Filter filter) const
{
    return samplers.at(filter);
}
[[nodiscard]] const vk::DescriptorSetLayout &
RenderResources::get_desc_set_layout(const std::string &name) const
{
    return desc_set_layouts.at(name);
}
[[nodiscard]] const Mesh &
RenderResources::get_mesh(const std::string &name) const
{
    return *meshes.at(name);
}
} // namespace kovra
