#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Material;

class ComputePass
{
  public:
    ComputePass(const vk::CommandBuffer &cmd)
      : cmd{ cmd }
    {
    }

    void set_material(std::shared_ptr<Material> material) noexcept;
    void set_push_constants(const std::span<const std::byte> &data) const;
    void set_desc_sets(
      uint32_t first_set,
      const std::vector<vk::DescriptorSet> &desc_sets,
      const std::vector<uint32_t> &dynamic_offsets
    ) const;
    // Dispatch compute work operations
    // x, y, z: number of workgroups to dispatch in each dimension
    void dispatch_workgroups(uint32_t x, uint32_t y, uint32_t z) const;

    [[nodiscard]] const vk::CommandBuffer &get_cmd() const { return cmd; }

  private:
    const vk::CommandBuffer &cmd;
    std::shared_ptr<Material> material;
};
} // namespace kovra
