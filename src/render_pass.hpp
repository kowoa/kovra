#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Material;

struct RenderPassDescriptor {
    std::vector<vk::RenderingAttachmentInfo> color_attachments;
    vk::RenderingAttachmentInfo depth_attachment;
    vk::Rect2D render_area;
};

class RenderPass {
  public:
    RenderPass(const RenderPassDescriptor &desc, const vk::CommandBuffer &cmd);
    ~RenderPass();

    void set_material(std::shared_ptr<Material> material) noexcept;
    void set_push_constants(const std::vector<uint8_t> &data) const;
    void set_desc_sets(
        uint32_t first_set, const std::vector<vk::DescriptorSet> &desc_sets,
        const std::vector<uint32_t> &dynamic_offsets) const;
    void set_viewport_scissor(uint32_t width, uint32_t height) const noexcept;

    [[nodiscard]] const vk::CommandBuffer &get_cmd() const { return cmd; }

  private:
    const vk::CommandBuffer &cmd;
    std::shared_ptr<Material> material;
};
} // namespace kovra
