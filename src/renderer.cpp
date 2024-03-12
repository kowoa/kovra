#include "renderer.hpp"
#include "physical_device.hpp"
#include <iostream>

namespace kovra {
Renderer::Renderer(SDL_Window *window) : context{window} {
    std::cout << "Renderer::Renderer()" << std::endl;
}
void Renderer::draw_frame() {
    std::cout << "Renderer::draw_frame()" << std::endl;
}
} // namespace kovra
