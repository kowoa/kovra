#pragma once

#include "shader.hpp"
#include "vertex.hpp"
#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Device;

class Material
{
  public:
    Material(
      vk::UniquePipeline pipeline,
      vk::UniquePipelineLayout pipeline_layout,
      const vk::PipelineBindPoint &&pipeline_bind_point
    )
      : pipeline(std::move(pipeline))
      , pipeline_layout(std::move(pipeline_layout))
      , pipeline_bind_point(std::move(pipeline_bind_point))
    {
    }
    Material(const Material &) = delete;
    Material &operator=(const Material &) = delete;
    Material(Material &&rhs) noexcept
      : pipeline(std::move(rhs.pipeline))
      , pipeline_layout(std::move(rhs.pipeline_layout))
      , pipeline_bind_point(std::move(rhs.pipeline_bind_point))
    {
    }
    Material &operator=(Material &&rhs) noexcept
    {
        if (this != &rhs) {
            pipeline = std::move(rhs.pipeline);
            pipeline_layout = std::move(rhs.pipeline_layout);
            pipeline_bind_point = std::move(rhs.pipeline_bind_point);
        }
        return *this;
    }

    void update_push_constants(
      vk::CommandBuffer cmd,
      vk::ShaderStageFlags stages,
      const std::span<const std::byte> &data
    ) const;
    void bind_pipeline(vk::CommandBuffer cmd) const;
    void bind_desc_sets(
      vk::CommandBuffer cmd,
      uint32_t first_set,
      const std::vector<vk::DescriptorSet> &desc_sets,
      const std::vector<uint32_t> &dynamic_offsets
    ) const;

  private:
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout pipeline_layout;
    vk::PipelineBindPoint pipeline_bind_point;
};

class GraphicsMaterialBuilder
{
  public:
    GraphicsMaterialBuilder();
    Material build(const vk::Device &device);

    GraphicsMaterialBuilder &set_shader(std::unique_ptr<GraphicsShader> shader);
    GraphicsMaterialBuilder &set_pipeline_layout(
      vk::UniquePipelineLayout pipeline_layout
    );
    GraphicsMaterialBuilder &set_input_topology(vk::PrimitiveTopology topology);
    GraphicsMaterialBuilder &set_polygon_mode(vk::PolygonMode polygon_mode);
    GraphicsMaterialBuilder &
    set_cull_mode(vk::CullModeFlags cull_mode, vk::FrontFace front_face);
    GraphicsMaterialBuilder &disable_multisampling();
    GraphicsMaterialBuilder &disable_blending();
    GraphicsMaterialBuilder &enable_alpha_blending();
    GraphicsMaterialBuilder &enable_additive_blending();
    GraphicsMaterialBuilder &set_color_attachment_format(vk::Format format);
    GraphicsMaterialBuilder &set_depth_attachment_format(vk::Format format);
    GraphicsMaterialBuilder &
    enable_depth_test(bool enable, vk::CompareOp op = vk::CompareOp::eAlways);
    GraphicsMaterialBuilder &set_vertex_input_desc(
      const VertexInputDescription &&desc
    );

  private:
    VertexInputDescription vertex_input_desc;
    vk::PipelineVertexInputStateCreateInfo vertex_input_ci;
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_ci;
    vk::PipelineRasterizationStateCreateInfo rasterization_ci;
    vk::PipelineColorBlendAttachmentState color_blend_attachment;
    vk::PipelineMultisampleStateCreateInfo multisample_ci;
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_ci;
    vk::PipelineRenderingCreateInfo rendering_ci;

    // Required fields for building a graphics material
    std::optional<std::unique_ptr<GraphicsShader>> shader;
    std::optional<vk::UniquePipelineLayout> pipeline_layout;
    std::optional<vk::Format> color_attachment_format;
    std::optional<vk::Format> depth_attachment_format;
};

class ComputeMaterialBuilder
{
  public:
    ComputeMaterialBuilder() = default;
    Material build(const vk::Device &device);

    ComputeMaterialBuilder &set_shader(std::unique_ptr<ComputeShader> shader);
    ComputeMaterialBuilder &set_pipeline_layout(
      vk::UniquePipelineLayout pipeline_layout
    );

  private:
    // Required fields for building a compute material
    std::optional<std::unique_ptr<ComputeShader>> shader;
    std::optional<vk::UniquePipelineLayout> pipeline_layout;
};
} // namespace kovra
