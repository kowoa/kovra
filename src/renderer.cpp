#include "renderer.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Renderer::Renderer(SDL_Window *window)
    : context{window}, swapchain{context, window}, frame_number{0} {
    spdlog::debug("Renderer::Renderer()");
}
Renderer::~Renderer() {
    spdlog::debug("Renderer::~Renderer()");
    // Wait until all frames have finished rendering
    for (const auto &frame : frames) {
        auto render_fence = frame->get_render_fence();
        if (const auto result = context.get_device().waitForFences(
                1, &render_fence, VK_TRUE, UINT64_MAX);
            result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for render fence");
        }
    }
}

void Renderer::draw_frame() {
    // get_current_frame().draw();
    frame_number++;
}
} // namespace kovra
