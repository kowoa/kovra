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

    // renderer->load_gltf("./assets/basicmesh.glb", "basicmesh");
    // renderer->load_gltf("./assets/structure.glb", "structure");
    renderer->load_gltf(
      "./assets/damaged-helmet/DamagedHelmet.glb", "DamagedHelmet"
    );
    renderer->load_gltf("./assets/boom-box/BoomBox.glb", "BoomBox");
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

        if (renderer->get_context().get_swapchain().is_dirty()) {
            renderer->get_context_mut().recreate_swapchain(window);
        }

        draw_imgui();

        std::vector<std::pair<std::string, glm::mat4>> objects_to_render{
            /*
                  { "DamagedHelmet",
                    glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f,
               0.0f)) }, { "DamagedHelmet", glm::translate(glm::mat4(1.0f),
               glm::vec3(-1.0f, 1.0f, 0.0f)) }, { "DamagedHelmet",
                    glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f,
               0.0f)) }, { "DamagedHelmet", glm::translate(glm::mat4(1.0f),
               glm::vec3(1.0f, 1.0f, 0.0f)) }
              */
            { "DamagedHelmet", glm::scale(glm::mat4(1.0f), glm::vec3(4.0f)) }
        };
        renderer->draw_frame(camera, objects_to_render);
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

    // Slider for render scale
    if (ImGui::SliderFloat("Render Scale", &render_scale, 0.3f, 1.0f)) {
        renderer->set_render_scale(render_scale);
    }

    // Renderer profiling stats
    const auto &stats = renderer->get_stats();
    ImGui::Begin("Profiling Stats");
    ImGui::Text("Frame time: %.2f ms", stats.frame_time);
    ImGui::Text("Triangle count: %d", stats.triangle_count);
    ImGui::Text("Draw call count: %d", stats.draw_call_count);
    ImGui::Text("Scene update time: %.2f ms", stats.scene_update_time);
    ImGui::Text(
      "Render objects draw time: %.2f ms", stats.render_objects_draw_time
    );
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

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");
    // Do not block compositing on X11
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
        throw std::runtime_error(
          std::format("SDL_Vulkan_LoadLibrary failed: {}", SDL_GetError())
        );
    }
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
    return std::make_unique<Renderer>(window, true);
}
} // namespace kovra
