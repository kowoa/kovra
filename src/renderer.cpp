#include "renderer.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Renderer::Renderer(SDL_Window *window)
    : context{window}, swapchain{context, window} {
    spdlog::debug("Renderer::Renderer()");
}
Renderer::~Renderer() { spdlog::debug("Renderer::~Renderer()"); }
void Renderer::draw_frame() {}
} // namespace kovra
