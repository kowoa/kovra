#include "pbr_material.hpp"
#include "descriptor.hpp"
#include "image.hpp"
#include "material.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"

namespace kovra {
PbrMaterial::PbrMaterial(
  const vk::Device &device,
  const vk::DescriptorSetLayout &scene_desc_layout,
  const vk::Format &color_attachment_format,
  const vk::Format &depth_attachment_format,
  const vk::SampleCountFlagBits &sample_count
)
  : desc_writer{ std::make_unique<DescriptorWriter>() }
{
    auto push_constant_range =
      vk::PushConstantRange{}
        .setStageFlags(
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
        )
        .setOffset(0)
        .setSize(sizeof(GpuPushConstants));

    // Create/get descriptor set layouts
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
        .add_binding(
          3, vk::DescriptorType::eCombinedImageSampler, vert_frag_stages
        )
        .add_binding(
          4, vk::DescriptorType::eCombinedImageSampler, vert_frag_stages
        )
        .build_unique(device);

    const auto layouts = std::array{ scene_desc_layout, material_layout.get() };

    opaque_material = std::make_shared<Material>(
      GraphicsMaterialBuilder{}
        .set_pipeline_layout(device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}
            .setSetLayouts(layouts)
            .setPushConstantRanges(push_constant_range)
        ))
        .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{ "pbr",
                                                                     device }))
        .set_color_attachment_format(color_attachment_format)
        .set_depth_attachment_format(depth_attachment_format)
        .set_multisampling(sample_count)
        .disable_blending()
        .build(device)
    );

    transparent_material = std::make_shared<Material>(
      GraphicsMaterialBuilder{}
        .set_pipeline_layout(device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}
            .setSetLayouts(layouts)
            .setPushConstantRanges(push_constant_range)
        ))
        .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{ "pbr",
                                                                     device }))
        .set_color_attachment_format(color_attachment_format)
        .set_depth_attachment_format(depth_attachment_format)
        .enable_additive_blending()
        .set_depth_test(true, vk::CompareOp::eLess)
        .set_multisampling(sample_count)
        .build(device)
    );
}

PbrMaterial::~PbrMaterial()
{
    desc_writer.reset();
    material_layout.reset();
    transparent_material.reset();
    opaque_material.reset();
}

MaterialInstance
PbrMaterial::create_material_instance(
  const PbrMaterialInstanceCreateInfo &info,
  const Device &device,
  DescriptorAllocator &desc_allocator
) const
{
    auto desc_set =
      desc_allocator.allocate(material_layout.get(), device.get());
    desc_writer->clear();
    desc_writer->write_buffer(
      0,
      info.material_buffer,
      sizeof(GpuPbrMaterialData),
      info.material_buffer_offset,
      vk::DescriptorType::eUniformBuffer
    );
    desc_writer->write_image(
      1,
      info.albedo_texture.get_view(),
      info.albedo_sampler,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    desc_writer->write_image(
      2,
      info.metal_rough_texture.get_view(),
      info.metal_rough_sampler,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    desc_writer->write_image(
      3,
      info.ambient_occlusion_texture.get_view(),
      info.ambient_occlusion_sampler,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    desc_writer->write_image(
      4,
      info.emissive_texture.get_view(),
      info.emissive_sampler,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    desc_writer->update_set(device.get(), desc_set);

    if (info.pass == MaterialPass::Opaque) {
        return MaterialInstance{ opaque_material, desc_set, info.pass };
    } else {
        return MaterialInstance{ transparent_material, desc_set, info.pass };
    }
}
}
