#include "pbr_material.hpp"
#include "descriptor.hpp"
#include "image.hpp"
#include "material.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"

namespace kovra {
PbrMaterial::PbrMaterial(const Renderer &renderer)
  : writer{ std::make_unique<DescriptorWriter>() }
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
        .build_unique(device);

    const auto layouts = std::array{ scene_layout, material_layout.get() };

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

MaterialInstance
PbrMaterial::create_material_instance(
  const PbrMaterialInstanceCreateInfo &info,
  const Device &device,
  DescriptorAllocator &desc_allocator
)
{
    auto desc_set =
      desc_allocator.allocate(material_layout.get(), device.get());
    writer->clear();
    writer->write_buffer(
      0,
      info.data_buffer,
      sizeof(GpuPbrMaterialData),
      info.data_buffer_offset,
      vk::DescriptorType::eUniformBuffer
    );
    writer->write_image(
      1,
      info.color_image.get_view(),
      info.color_image.get_sampler(),
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    writer->write_image(
      2,
      info.metal_rough_image.get_view(),
      info.metal_rough_image.get_sampler(),
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    writer->update_set(device.get(), desc_set);

    if (info.pass == MaterialPass::Opaque) {
        return MaterialInstance{ *opaque_material, desc_set, info.pass };
    } else {
        return MaterialInstance{ *transparent_material, desc_set, info.pass };
    }
}
}
