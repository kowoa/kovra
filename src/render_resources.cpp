#include "render_resources.hpp"
#include "device.hpp"
#include "material.hpp"

namespace kovra {
RenderResources::RenderResources(std::shared_ptr<Device> device)
    : device{std::move(device)} {}
RenderResources::~RenderResources() {
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

void RenderResources::add_material(
    const std::string &name, const Material &material) {
    materials.emplace(name, material);
}
void RenderResources::add_sampler(
    vk::Filter filter, const vk::Sampler &sampler) {
    samplers.emplace(filter, sampler);
}
void RenderResources::add_desc_set_layout(
    const std::string &name, const vk::DescriptorSetLayout &desc_set_layout) {
    desc_set_layouts.emplace(name, desc_set_layout);
}
[[nodiscard]] const Material &
RenderResources::get_material(const std::string &name) const {
    return materials.at(name);
}
[[nodiscard]] const vk::Sampler &
RenderResources::get_sampler(vk::Filter filter) const {
    return samplers.at(filter);
}
[[nodiscard]] const vk::DescriptorSetLayout &
RenderResources::get_desc_set_layout(const std::string &name) const {
    return desc_set_layouts.at(name);
}
} // namespace kovra
