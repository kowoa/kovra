#include <SDL2/SDL.h>

#include "app.hpp"
#include "renderer.hpp"

namespace kovra {
App::App() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(
            std::format("SDL_Init failed: {}", SDL_GetError()));
    }
    SDL_Window *window = SDL_CreateWindow(
        "Kovra", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900,
        SDL_WINDOW_VULKAN);
    if (!window) {
        throw std::runtime_error(
            std::format("SDL_CreateWindow failed: {}", SDL_GetError()));
    }
    Renderer renderer = Renderer{window};
}
void App::run() {
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }
}
} // namespace kovra
