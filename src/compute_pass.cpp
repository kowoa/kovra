#include "compute_pass.hpp"
#include "material.hpp"
#include "utils.hpp"

namespace kovra {
void
ComputePass::set_material(std::shared_ptr<Material> material) noexcept
{
    material->bind_pipeline(cmd);
    this->material = material;
}
void
ComputePass::set_push_constants(const std::span<const std::byte> &data) const
{
    if (!material) {
        throw std::runtime_error("Material not set");
        return;
    }
    material->update_push_constants(
      cmd, vk::ShaderStageFlagBits::eCompute, data
    );
}
void
ComputePass::set_desc_sets(
  uint32_t first_set,
  const std::vector<vk::DescriptorSet> &desc_sets,
  const std::vector<uint32_t> &dynamic_offsets
) const
{
    if (!material) {
        throw std::runtime_error("Material not set");
        return;
    }
    material->bind_desc_sets(cmd, first_set, desc_sets, dynamic_offsets);
}
void
ComputePass::dispatch_workgroups(uint32_t x, uint32_t y, uint32_t z) const
{
    cmd.dispatch(x, y, z);
}
void
ComputePass::transition_image_layout(
  const vk::Image &image,
  vk::ImageAspectFlagBits aspect,
  vk::ImageLayout old_layout,
  vk::ImageLayout new_layout
) const
{
    utils::transition_image_layout(cmd, image, aspect, old_layout, new_layout);
}
} // namespace kovra
