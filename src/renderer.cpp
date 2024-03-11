#include "renderer.hpp"
#include <iostream>

namespace kovra {
Renderer::Renderer(SDL_Window *window) {
    std::cout << "Renderer::Renderer()" << std::endl;
}
void Renderer::draw_frame() {
    std::cout << "Renderer::draw_frame()" << std::endl;
}
} // namespace kovra
