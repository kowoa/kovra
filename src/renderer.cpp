#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "descriptor.hpp"
#include "renderer.hpp"

#include "spdlog/spdlog.h"

namespace kovra {
void init_desc_set_layouts(
    const vk::Device &device,
    std::unordered_map<std::string, vk::DescriptorSetLayout>
        &desc_sets_layouts);
vk::UniqueSampler create_sampler(vk::Filter filter, const vk::Device &device);

Renderer::Renderer(SDL_Window *window)
    : context{std::make_unique<Context>(window)}, frame_number{0} {
    spdlog::debug("Renderer::Renderer()");

    // Create frames
    frames.reserve(FRAME_OVERLAP);
    for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
        frames.emplace_back(
            std::make_unique<Frame>(*context->get_device_owned()));
    }

    // Create descriptor set layouts
    init_desc_set_layouts(context->get_device(), desc_set_layouts);

    // Create samplers
    samplers.emplace(
        vk::Filter::eNearest,
        create_sampler(vk::Filter::eNearest, context->get_device()));
    samplers.emplace(
        vk::Filter::eLinear,
        create_sampler(vk::Filter::eLinear, context->get_device()));

    // Create background image
    auto swapchain_extent = context->get_swapchain().get_extent();
    background_image = context->get_device_owned()->create_storage_image(
        swapchain_extent.width, swapchain_extent.height,
        samplers.at(vk::Filter::eNearest).get());
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

    // Destroy samplers
    for (auto &[_, sampler] : samplers) {
        sampler.reset();
    }
    samplers.clear();

    // Destroy descriptor set layouts
    for (auto &[name, desc_set_layout] : desc_set_layouts) {
        spdlog::debug("Destroying descriptor set layout: {}", name);
        context->get_device().destroyDescriptorSetLayout(desc_set_layout);
    }
    desc_set_layouts.clear();

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
        .desc_set_layouts = desc_set_layouts,
        .background_image = background_image};

    get_current_frame().draw(draw_ctx);
    frame_number++;
}

vk::UniqueSampler create_sampler(vk::Filter filter, const vk::Device &device) {
    return device.createSamplerUnique(
        vk::SamplerCreateInfo{}
            .setMagFilter(filter)
            .setMinFilter(filter)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat));
}

void init_desc_set_layouts(
    const vk::Device &device,
    std::unordered_map<std::string, vk::DescriptorSetLayout>
        &desc_set_layouts) {
    // Create a descriptor set layout for the scene buffer
    auto scene = DescriptorSetLayoutBuilder{}
                     .add_binding(
                         0, vk::DescriptorType::eUniformBuffer,
                         vk::ShaderStageFlagBits::eVertex |
                             vk::ShaderStageFlagBits::eFragment)
                     .build(device);
    desc_set_layouts.emplace("scene", std::move(scene));
}
} // namespace kovra
