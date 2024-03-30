#include "app.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
#include "spdlog/spdlog.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

namespace kovra {
SDL_Window *
create_window();
std::unique_ptr<Renderer>
create_renderer(SDL_Window *window);

App::App()
  : window{ create_window() }
  , renderer{ create_renderer(window) }
{
    spdlog::debug("App::App()");

    renderer->load_gltf("./assets/basicmesh.glb");
}
App::~App()
{
    spdlog::debug("App::~App()");
    // Destroy the renderer before the window
    renderer.reset();
    // Destroy the window
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void
App::run()
{
    spdlog::debug("App::run()");

    SDL_Event event;
    bool close_requested = false;
    bool rendering_stopped = false;

    while (!close_requested) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    close_requested = true;
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                        rendering_stopped = true;
                    }
                    if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
                        rendering_stopped = false;
                    }
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        close_requested = true;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        camera_movable = true;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        camera_movable = false;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    camera.mouse_zoom(event.wheel.preciseY);
                    break;
                case SDL_MOUSEMOTION:
                    glm::vec2 curr_mouse_pos{ event.motion.x, event.motion.y };
                    if (camera_movable) {
                        camera.mouse_rotate(
                          prev_mouse_pos, curr_mouse_pos, 1600.0f, 900.0f
                        );
                    }
                    prev_mouse_pos = curr_mouse_pos;
                    break;
            }

            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        if (rendering_stopped) {
            // Throttle speed to avoid endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        draw_imgui();

        renderer->draw_frame(camera, window);
    }
}

void
App::draw_imgui()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Calculate FPS
    float curr_frame_time = SDL_GetTicks() / 1000.0f;
    float elapsed = std::max(curr_frame_time - prev_frame_time, 0.0001f);
    double frame_count_time = frame_count_since_last_second / elapsed;
    if (elapsed > 1.0f) {
        frame_count_since_last_second = 0;
        prev_frame_time = curr_frame_time;
        fps = frame_count_time;
    }
    frame_count_since_last_second++;

    // Show FPS
    ImGui::SetNextWindowPos({ 10, 10 });
    ImGui::SetNextWindowSize({ 100, 20 });
    ImGui::Begin(
      "FPS",
      nullptr,
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground
    );
    ImGui::Text("FPS: %.2f", fps);
    ImGui::End();

    ImGui::Render();
}

SDL_Window *
create_window()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(
          std::format("SDL_Init failed: {}", SDL_GetError())
        );
    }
    SDL_Vulkan_LoadLibrary(nullptr);
    auto window = SDL_CreateWindow(
      "Kovra",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      1600,
      900,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        throw std::runtime_error(
          std::format("SDL_CreateWindow failed: {}", SDL_GetError())
        );
    }
    return window;
}

std::unique_ptr<Renderer>
create_renderer(SDL_Window *window)
{
    return std::make_unique<Renderer>(window);
}
} // namespace kovra
