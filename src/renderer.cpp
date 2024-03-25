#include "material.hpp"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "descriptor.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"

#include "spdlog/spdlog.h"

namespace kovra {
void init_desc_set_layouts(
    const vk::Device &device, RenderResources &resources);
void init_materials(const vk::Device &device, RenderResources &resources);
vk::Sampler create_sampler(vk::Filter filter, const vk::Device &device);

Renderer::Renderer(SDL_Window *window)
    : context{std::make_unique<Context>(window)}, frame_number{0},
      render_resources{
          std::make_shared<RenderResources>(context->get_device_owned())} {
    spdlog::debug("Renderer::Renderer()");

    // Create frames
    frames.reserve(FRAME_OVERLAP);
    for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
        frames.emplace_back(
            std::make_unique<Frame>(*context->get_device_owned()));
    }

    // Create descriptor set layouts
    init_desc_set_layouts(context->get_device(), *render_resources);

    // Create materials
    init_materials(context->get_device(), *render_resources);

    // Create samplers
    render_resources->add_sampler(
        vk::Filter::eNearest,
        create_sampler(vk::Filter::eNearest, context->get_device()));
    render_resources->add_sampler(
        vk::Filter::eLinear,
        create_sampler(vk::Filter::eLinear, context->get_device()));

    // Create background image
    auto swapchain_extent = context->get_swapchain().get_extent();
    background_image = context->get_device_owned()->create_storage_image(
        swapchain_extent.width, swapchain_extent.height,
        render_resources->get_sampler(vk::Filter::eNearest));
}

Renderer::~Renderer() {
    spdlog::debug("Renderer::~Renderer()");
    // Wait until all frames have finished rendering
    for (const auto &frame : frames) {
        auto render_fence = frame->get_render_fence();
        if (const auto result = context->get_device().waitForFences(
                1, &render_fence, VK_TRUE, UINT64_MAX);
            result != vk::Result::eSuccess) {
            spdlog::error(
                "Failed to wait for render fence: {}", vk::to_string(result));
        }
    }

    background_image.reset();
    render_resources.reset();

    // Destroy frames
    for (auto &frame : frames) {
        frame.reset();
    }
    frames.clear();

    context.reset();
}

void Renderer::draw_frame(const Camera &camera) {
    auto draw_ctx = DrawContext{
        .device = context->get_device_owned(),
        .swapchain = context->get_swapchain_owned(),
        .frame_number = frame_number,
        .camera = camera,
        .render_resources = render_resources,
        .background_image = background_image};

    get_current_frame().draw(draw_ctx);
    frame_number++;
}

vk::Sampler create_sampler(vk::Filter filter, const vk::Device &device) {
    return device.createSampler(
        vk::SamplerCreateInfo{}
            .setMagFilter(filter)
            .setMinFilter(filter)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat));
}

void init_desc_set_layouts(
    const vk::Device &device, RenderResources &resources) {
    // Create a descriptor set layout for the scene buffer
    auto scene = DescriptorSetLayoutBuilder{}
                     .add_binding(
                         0, vk::DescriptorType::eUniformBuffer,
                         vk::ShaderStageFlagBits::eVertex |
                             vk::ShaderStageFlagBits::eFragment)
                     .build(device);
    resources.add_desc_set_layout("scene", std::move(scene));

    // Create a descriptor set layout for the background image
    auto background = DescriptorSetLayoutBuilder{}
                          .add_binding(
                              0, vk::DescriptorType::eStorageImage,
                              vk::ShaderStageFlagBits::eCompute)
                          .build(device);
    resources.add_desc_set_layout("background", std::move(background));
}

void init_materials(const vk::Device &device, RenderResources &resources) {
    // Create a material for the background image
    {
        auto desc_set_layouts =
            std::array{resources.get_desc_set_layout("background")};
        auto pipeline_layout = device.createPipelineLayoutUnique(
            vk::PipelineLayoutCreateInfo{}.setSetLayouts(desc_set_layouts));
        auto background = ComputeMaterialBuilder{}
                              .set_pipeline_layout(std::move(pipeline_layout))
                              .set_shader(std::make_unique<ComputeShader>(
                                  ComputeShader{"sky", device}))
                              .build(device);
        resources.add_material("sky", std::move(background));
    }
}
} // namespace kovra
