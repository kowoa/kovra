#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
struct RenderPassDescriptor {
    std::vector<vk::RenderingAttachmentInfo> color_attachments;
    vk::RenderingAttachmentInfo depth_attachment;
    vk::Rect2D render_area;
};

class RenderPass {
  public:
    RenderPass(const RenderPassDescriptor &desc, const vk::CommandBuffer &cmd);

  private:
    const vk::CommandBuffer &cmd;
};
} // namespace kovra
