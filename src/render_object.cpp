#include "render_object.hpp"
#include "descriptor.hpp"
#include "material.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"
#include "shader.hpp"

namespace kovra {
void
GltfMetallicRoughness::build_materials(const Renderer &renderer)
{
    const vk::Device &device = renderer.get_context().get_device().get();

    auto push_constant_range =
      vk::PushConstantRange{}
        .setStageFlags(vk::ShaderStageFlagBits::eVertex)
        .setOffset(0)
        .setSize(sizeof(GpuPushConstants));

    // Create/get descriptor set layouts
    auto scene_layout =
      renderer.get_render_resources().get_desc_set_layout("scene");

    const auto vert_frag_stages =
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    material_layout =
      DescriptorSetLayoutBuilder{}
        .add_binding(0, vk::DescriptorType::eUniformBuffer, vert_frag_stages)
        .add_binding(
          1, vk::DescriptorType::eCombinedImageSampler, vert_frag_stages
        )
        .add_binding(
          2, vk::DescriptorType::eCombinedImageSampler, vert_frag_stages
        )
        .build(device);

    const auto layouts = std::array{ scene_layout, material_layout };

    opaque_material = std::make_unique<Material>(
      GraphicsMaterialBuilder{}
        .set_pipeline_layout(device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}
            .setSetLayouts(layouts)
            .setPushConstantRanges(push_constant_range)
        ))
        .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{
          "mesh-gltf", device }))
        .set_color_attachment_format(renderer.get_draw_image().get_format())
        .set_depth_attachment_format(
          renderer.get_context().get_swapchain().get_depth_image().get_format()
        )
        .disable_multisampling()
        .disable_blending()
        .build(device)
    );

    transparent_material = std::make_unique<Material>(
      GraphicsMaterialBuilder{}
        .set_pipeline_layout(device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}
            .setSetLayouts(layouts)
            .setPushConstantRanges(push_constant_range)
        ))
        .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{
          "mesh-gltf", device }))
        .set_color_attachment_format(renderer.get_draw_image().get_format())
        .set_depth_attachment_format(
          renderer.get_context().get_swapchain().get_depth_image().get_format()
        )
        .enable_additive_blending()
        .enable_depth_test(false, vk::CompareOp::eGreaterOrEqual)
        .build(device)
    );
}
}
