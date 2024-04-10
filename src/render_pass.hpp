#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Material;
class GpuBuffer;

struct RenderPassCreateInfo
{
    std::vector<vk::RenderingAttachmentInfo> color_attachments;
    vk::RenderingAttachmentInfo depth_attachment;
    vk::Rect2D render_area;
};

class RenderPass
{
  public:
    RenderPass(const RenderPassCreateInfo &info, const vk::CommandBuffer &cmd);
    ~RenderPass();

    void set_material(std::shared_ptr<Material> material) noexcept;
    void set_push_constants(const std::span<const std::byte> &data) const;
    void set_desc_sets(
      uint32_t first_set,
      const std::vector<vk::DescriptorSet> &desc_sets,
      const std::vector<uint32_t> &dynamic_offsets = {}
    ) const;
    void set_viewport_scissor(uint32_t width, uint32_t height) const noexcept;
    void set_index_buffer(const vk::Buffer &index_buffer) noexcept;

    void draw(
      uint32_t vertex_count,
      uint32_t instance_count,
      uint32_t first_vertex,
      uint32_t first_instance
    ) const noexcept;
    void draw_indexed(
      uint32_t index_count,
      uint32_t instance_count,
      uint32_t first_index,
      int32_t vertex_offset,
      uint32_t first_instance
    ) const;

    [[nodiscard]] const vk::CommandBuffer &get_cmd() const { return cmd; }

  private:
    const vk::CommandBuffer &cmd;
    std::shared_ptr<Material> material;
};
} // namespace kovra
