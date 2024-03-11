#include "renderer.hpp"
#include <iostream>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace kovra {
Renderer::Renderer(SDL_Window *window) : context{window} {
    std::cout << "Renderer::Renderer()" << std::endl;
}
void Renderer::draw_frame() {
    std::cout << "Renderer::draw_frame()" << std::endl;
}
} // namespace kovra
