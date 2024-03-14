#include "render_pass.hpp"

namespace kovra {

RenderPass::RenderPass(
    const RenderPassDescriptor &desc, const vk::CommandBuffer &cmd)
    : cmd{cmd} {
    auto rendering_info = vk::RenderingInfo{}
                              .setColorAttachments(desc.color_attachments)
                              .setPDepthAttachment(&desc.depth_attachment)
                              .setRenderArea(desc.render_area)
                              .setLayerCount(1);
    cmd.beginRendering(rendering_info);
}
} // namespace kovra
