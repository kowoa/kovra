#include "renderer.hpp"
#include "descriptor.hpp"
#include "frame.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
void init_desc_set_layouts(
    const vk::Device &device,
    std::unordered_map<std::string, vk::UniqueDescriptorSetLayout>
        &desc_sets_layouts);

Renderer::Renderer(SDL_Window *window)
    : context{std::make_unique<Context>(window)}, frame_number{0} {
    spdlog::debug("Renderer::Renderer()");

    // Create frames
    frames.reserve(FRAME_OVERLAP);
    for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
        frames.emplace_back(
            std::make_unique<Frame>(*context->get_device_owned()));
    }

    init_desc_set_layouts(context->get_device(), desc_set_layouts);
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

    for (auto &[name, desc_set_layout] : desc_set_layouts) {
        desc_set_layout.reset();
    }
    desc_set_layouts.clear();

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
        .desc_set_layouts = desc_set_layouts};

    spdlog::debug("Before drawing frame");
    get_current_frame().draw(draw_ctx);
    spdlog::debug("After drawing frame");
    frame_number++;
}

void init_desc_set_layouts(
    const vk::Device &device,
    std::unordered_map<std::string, vk::UniqueDescriptorSetLayout>
        &desc_set_layouts) {
    // Create a descriptor set layout for the scene buffer
    auto scene = DescriptorSetLayoutBuilder{}
                     .add_binding(
                         0, vk::DescriptorType::eUniformBuffer,
                         vk::ShaderStageFlagBits::eVertex |
                             vk::ShaderStageFlagBits::eFragment)
                     .build(device);
    desc_set_layouts["scene"] = std::move(scene);
}
} // namespace kovra
