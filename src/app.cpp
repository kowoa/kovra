#include "app.hpp"
#include "SDL.h"
#include "spdlog/spdlog.h"

namespace kovra {
SDL_Window *create_window();
std::unique_ptr<Renderer> create_renderer(SDL_Window *window);

App::App() : window{create_window()}, renderer{create_renderer(window)} {
    spdlog::debug("App::App()");
}
App::~App() {
    spdlog::debug("App::~App()");
    // Destroy the renderer before the window
    renderer.reset();
    // Destroy the window
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void App::run() {
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }

            renderer->draw_frame(camera);
        }
    }
}

SDL_Window *create_window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(
            std::format("SDL_Init failed: {}", SDL_GetError()));
    }
    auto window = SDL_CreateWindow(
        "Kovra", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900,
        SDL_WINDOW_VULKAN);
    if (!window) {
        throw std::runtime_error(
            std::format("SDL_CreateWindow failed: {}", SDL_GetError()));
    }
    return window;
}

std::unique_ptr<Renderer> create_renderer(SDL_Window *window) {
    return std::make_unique<Renderer>(window);
}
} // namespace kovra
