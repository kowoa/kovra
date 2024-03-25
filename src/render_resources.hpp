#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Device;
class Material;

class RenderResources {
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
        const vk::DescriptorSetLayout &&desc_set_layout);

    [[nodiscard]] const Material &get_material(const std::string &name) const;
    [[nodiscard]] std::shared_ptr<Material>
    get_material_owned(const std::string &name) const;
    [[nodiscard]] const vk::Sampler &get_sampler(vk::Filter filter) const;
    [[nodiscard]] const vk::DescriptorSetLayout &
    get_desc_set_layout(const std::string &name) const;

  private:
    std::shared_ptr<Device> device;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::unordered_map<vk::Filter, vk::Sampler> samplers;
    std::unordered_map<std::string, vk::DescriptorSetLayout> desc_set_layouts;
};
} // namespace kovra
