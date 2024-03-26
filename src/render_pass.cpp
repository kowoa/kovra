#include "render_pass.hpp"
#include "material.hpp"

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

RenderPass::~RenderPass() {
    cmd.endRendering();
    material.reset();
}

void RenderPass::set_material(std::shared_ptr<Material> material) noexcept {
    material->bind_pipeline(cmd);
    this->material = material;
}
void RenderPass::set_push_constants(const std::vector<uint8_t> &data) const {
    if (!material) {
        throw std::runtime_error("Material not set");
        return;
    }
    material->update_push_constants(
        cmd,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        data);
}
void RenderPass::set_desc_sets(
    uint32_t first_set, const std::vector<vk::DescriptorSet> &desc_sets,
    const std::vector<uint32_t> &dynamic_offsets) const {
    if (!material) {
        throw std::runtime_error("Material not set");
        return;
    }
    material->bind_desc_sets(cmd, first_set, desc_sets, dynamic_offsets);
}
void RenderPass::set_viewport_scissor(
    uint32_t width, uint32_t height) const noexcept {
    cmd.setViewport(
        0, vk::Viewport{}
               .setWidth(static_cast<float>(width))
               .setHeight(static_cast<float>(height))
               .setMinDepth(0.0f)
               .setMaxDepth(1.0f));
    cmd.setScissor(
        0, vk::Rect2D{}.setOffset({0, 0}).setExtent({width, height}));
}
} // namespace kovra
