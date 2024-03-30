#include "material.hpp"
#include "spdlog/spdlog.h"
#include <vulkan/vulkan.hpp>

namespace kovra {
void
Material::update_push_constants(
  vk::CommandBuffer cmd,
  vk::ShaderStageFlags stages,
  const std::span<const std::byte> &data
) const
{
    cmd.pushConstants(
      pipeline_layout.get(),
      stages,
      0,
      data.size() * sizeof(uint8_t),
      data.data()
    );
}
void
Material::bind_pipeline(vk::CommandBuffer cmd) const
{
    cmd.bindPipeline(pipeline_bind_point, pipeline.get());
}
void
Material::bind_desc_sets(
  vk::CommandBuffer cmd,
  uint32_t first_set,
  const std::vector<vk::DescriptorSet> &desc_sets,
  const std::vector<uint32_t> &dynamic_offsets
) const
{
    cmd.bindDescriptorSets(
      pipeline_bind_point,
      pipeline_layout.get(),
      first_set,
      desc_sets,
      dynamic_offsets
    );
}

GraphicsMaterialBuilder::GraphicsMaterialBuilder()
  : vertex_input_desc{}
  , vertex_input_ci{}
  , input_assembly_ci{ vk::PipelineInputAssemblyStateCreateInfo{}
                         .setTopology(vk::PrimitiveTopology::eTriangleList)
                         .setPrimitiveRestartEnable(VK_FALSE) }
  , rasterization_ci{ vk::PipelineRasterizationStateCreateInfo{}
                        .setDepthClampEnable(VK_FALSE)
                        .setRasterizerDiscardEnable(VK_FALSE)
                        .setPolygonMode(vk::PolygonMode::eFill)
                        .setLineWidth(1.0f)
                        .setCullMode(vk::CullModeFlagBits::eNone)
                        .setFrontFace(vk::FrontFace::eCounterClockwise)
                        .setDepthBiasEnable(VK_FALSE)
                        .setDepthBiasConstantFactor(0.0f)
                        .setDepthBiasClamp(0.0f)
                        .setDepthBiasSlopeFactor(0.0f) }
  , color_blend_attachment{ vk::PipelineColorBlendAttachmentState{}
                              .setColorWriteMask(
                                vk::ColorComponentFlagBits::eR |
                                vk::ColorComponentFlagBits::eG |
                                vk::ColorComponentFlagBits::eB |
                                vk::ColorComponentFlagBits::eA
                              )
                              .setBlendEnable(VK_TRUE)
                              .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha
                              )
                              .setDstColorBlendFactor(
                                vk::BlendFactor::eOneMinusSrcAlpha
                              )
                              .setColorBlendOp(vk::BlendOp::eAdd)
                              .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                              .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                              .setAlphaBlendOp(vk::BlendOp::eAdd) }
  , multisample_ci{ vk::PipelineMultisampleStateCreateInfo{}
                      .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                      .setSampleShadingEnable(VK_FALSE)
                      .setMinSampleShading(1.0f)
                      .setPSampleMask(nullptr)
                      .setAlphaToCoverageEnable(VK_FALSE)
                      .setAlphaToOneEnable(VK_FALSE) }
  , depth_stencil_ci{ vk::PipelineDepthStencilStateCreateInfo{}
                        .setDepthTestEnable(VK_TRUE)
                        .setDepthWriteEnable(VK_TRUE)
                        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                        .setDepthBoundsTestEnable(VK_FALSE)
                        .setStencilTestEnable(VK_FALSE)
                        .setMinDepthBounds(0.0f)
                        .setMaxDepthBounds(1.0f) }
  , rendering_ci{ vk::PipelineRenderingCreateInfo{} }
{
}

Material
GraphicsMaterialBuilder::build(const vk::Device &device)
{
    if (!shader.has_value() || !pipeline_layout.has_value() ||
        !color_attachment_format.has_value() ||
        !depth_attachment_format.has_value()) {
        throw std::runtime_error(
          "GraphicsMaterialBuilder: missing required fields"
        );
    }

    constexpr const char *shader_main_fn_name = "main";
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = {
        vk::PipelineShaderStageCreateInfo{}
          .setStage(vk::ShaderStageFlagBits::eVertex)
          .setModule(shader->get()->get_vert_shader_mod())
          .setPName(shader_main_fn_name),
        vk::PipelineShaderStageCreateInfo{}
          .setStage(vk::ShaderStageFlagBits::eFragment)
          .setModule(shader->get()->get_frag_shader_mod())
          .setPName(shader_main_fn_name)
    };

    auto viewport_state_ci{ vk::PipelineViewportStateCreateInfo{}
                              .setViewportCount(1)
                              .setScissorCount(1) };

    auto color_blend_ci{ vk::PipelineColorBlendStateCreateInfo{}
                           .setLogicOp(vk::LogicOp::eCopy)
                           .setLogicOpEnable(vk::False)
                           .setAttachmentCount(1)
                           .setPAttachments(&color_blend_attachment) };

    std::vector<vk::DynamicState> dynamic_states = {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
    };
    auto dynamic_ci =
      vk::PipelineDynamicStateCreateInfo{}.setDynamicStates(dynamic_states);

    auto pipeline_ci = vk::GraphicsPipelineCreateInfo{}
                         .setPNext(&rendering_ci)
                         .setStages(shader_stages)
                         .setLayout(pipeline_layout->get())
                         .setPVertexInputState(&vertex_input_ci)
                         .setPInputAssemblyState(&input_assembly_ci)
                         .setPViewportState(&viewport_state_ci)
                         .setPRasterizationState(&rasterization_ci)
                         .setPMultisampleState(&multisample_ci)
                         .setPColorBlendState(&color_blend_ci)
                         .setPDepthStencilState(&depth_stencil_ci)
                         .setPDynamicState(&dynamic_ci);

    auto pipeline = device.createGraphicsPipelineUnique(nullptr, pipeline_ci);
    if (pipeline.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    // Destroy shader after pipeline creation
    shader.reset();

    return Material{ std::move(pipeline.value),
                     std::move(*pipeline_layout),
                     vk::PipelineBindPoint::eGraphics };
}

GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_shader(std::unique_ptr<GraphicsShader> shader)
{
    // Destroy previous shader if it exists
    if (this->shader.has_value()) {
        this->shader.reset();
    }
    this->shader = std::move(shader);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_pipeline_layout(
  vk::UniquePipelineLayout pipeline_layout
)
{
    // Destroy previous pipeline layout if it exists
    if (this->pipeline_layout.has_value()) {
        this->pipeline_layout.reset();
    }
    this->pipeline_layout = std::move(pipeline_layout);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_input_topology(vk::PrimitiveTopology topology)
{
    input_assembly_ci.setTopology(topology);
    input_assembly_ci.setPrimitiveRestartEnable(vk::False);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_polygon_mode(vk::PolygonMode polygon_mode)
{
    rasterization_ci.setPolygonMode(polygon_mode);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_cull_mode(
  vk::CullModeFlags cull_mode,
  vk::FrontFace front_face
)
{
    rasterization_ci.setCullMode(cull_mode);
    rasterization_ci.setFrontFace(front_face);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::disable_multisampling()
{
    multisample_ci.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    multisample_ci.setSampleShadingEnable(vk::False);
    multisample_ci.setMinSampleShading(1.0f);
    multisample_ci.setPSampleMask(nullptr);
    multisample_ci.setAlphaToCoverageEnable(vk::False);
    multisample_ci.setAlphaToOneEnable(vk::False);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::disable_blending()
{
    color_blend_attachment.setColorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    color_blend_attachment.setBlendEnable(VK_FALSE);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::enable_alpha_blending()
{
    color_blend_attachment.setColorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    color_blend_attachment.setBlendEnable(vk::True);
    color_blend_attachment.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    color_blend_attachment.setDstColorBlendFactor(
      vk::BlendFactor::eOneMinusSrcAlpha
    );
    color_blend_attachment.setColorBlendOp(vk::BlendOp::eAdd);
    color_blend_attachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    color_blend_attachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    color_blend_attachment.setAlphaBlendOp(vk::BlendOp::eAdd);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::enable_additive_blending()
{
    color_blend_attachment.setColorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    color_blend_attachment.setBlendEnable(vk::True);
    color_blend_attachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
    color_blend_attachment.setDstColorBlendFactor(vk::BlendFactor::eDstAlpha);
    color_blend_attachment.setColorBlendOp(vk::BlendOp::eAdd);
    color_blend_attachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    color_blend_attachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    color_blend_attachment.setAlphaBlendOp(vk::BlendOp::eAdd);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_color_attachment_format(vk::Format format)
{
    color_attachment_format = format;
    rendering_ci.setColorAttachmentCount(1);
    rendering_ci.setPColorAttachmentFormats(&color_attachment_format.value());
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_depth_attachment_format(vk::Format format)
{
    depth_attachment_format = format;
    rendering_ci.setDepthAttachmentFormat(depth_attachment_format.value());
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::enable_depth_test(bool enable)
{
    depth_stencil_ci.setDepthTestEnable(enable);
    depth_stencil_ci.setDepthWriteEnable(enable);
    depth_stencil_ci.setDepthCompareOp(
      enable ? vk::CompareOp::eLessOrEqual : vk::CompareOp::eAlways
    );
    depth_stencil_ci.setMinDepthBounds(0.0f);
    depth_stencil_ci.setMaxDepthBounds(1.0f);
    return *this;
}
GraphicsMaterialBuilder &
GraphicsMaterialBuilder::set_vertex_input_desc(
  const VertexInputDescription &&desc
)
{
    vertex_input_desc = std::move(desc);
    vertex_input_ci =
      vk::PipelineVertexInputStateCreateInfo{}
        .setVertexAttributeDescriptions(vertex_input_desc.attributes)
        .setVertexBindingDescriptions(vertex_input_desc.bindings)
        .setFlags(vertex_input_desc.flags);
    return *this;
}

Material
ComputeMaterialBuilder::build(const vk::Device &device)
{
    if (!shader.has_value() || !pipeline_layout.has_value()) {
        throw std::runtime_error(
          "ComputeMaterialBuilder: missing required fields"
        );
    }

    constexpr const char *shader_main_fn_name = "main";
    auto shader_stage = vk::PipelineShaderStageCreateInfo{}
                          .setStage(vk::ShaderStageFlagBits::eCompute)
                          .setModule((*shader)->get_shader_mod())
                          .setPName(shader_main_fn_name);

    auto pipeline = device.createComputePipelineUnique(
      nullptr,
      vk::ComputePipelineCreateInfo{}
        .setStage(shader_stage)
        .setLayout(pipeline_layout->get())
    );
    if (pipeline.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create compute pipeline");
    }

    shader.reset();

    return Material{ std::move(pipeline.value),
                     std::move(*pipeline_layout),
                     vk::PipelineBindPoint::eCompute };
}

ComputeMaterialBuilder &
ComputeMaterialBuilder::set_shader(std::unique_ptr<ComputeShader> shader)
{
    // Destroy previous shader if it exists
    if (this->shader.has_value()) {
        this->shader.reset();
    }
    this->shader = std::move(shader);
    return *this;
}
ComputeMaterialBuilder &
ComputeMaterialBuilder::set_pipeline_layout(
  vk::UniquePipelineLayout pipeline_layout
)
{
    // Destroy previous pipeline layout if it exists
    if (this->pipeline_layout.has_value()) {
        this->pipeline_layout.reset();
    }
    this->pipeline_layout = std::move(pipeline_layout);
    return *this;
}
} // namespace kovra
