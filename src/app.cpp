#include "app.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
#include "spdlog/spdlog.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

namespace kovra {
SDL_Window *create_window();
std::unique_ptr<Renderer> create_renderer(SDL_Window *window);

App::App() : window{create_window()}, renderer{create_renderer(window)} {
    spdlog::debug("App::App()");
    init_imgui(*renderer);
}
App::~App() {
    spdlog::debug("App::~App()");
    // Destroy ImGui
    vkDestroyDescriptorPool(
        renderer->get_context().get_device(), imgui_pool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    // Destroy the renderer before the window
    renderer.reset();
    // Destroy the window
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void App::run() {
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
                glm::vec2 curr_mouse_pos{event.motion.x, event.motion.y};
                if (camera_movable) {
                    camera.mouse_rotate(
                        prev_mouse_pos, curr_mouse_pos, 1600.0f, 900.0f);
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

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        renderer->draw_frame(camera);
    }
}

void App::init_imgui(const Renderer &renderer) {
    const Context &renderer_ctx{renderer.get_context()};
    const std::shared_ptr<Device> &device{renderer_ctx.get_device_owned()};

    // Create descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(
            renderer_ctx.get_device(), &pool_info, nullptr, &imgui_pool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool for ImGui");
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForVulkan(window);
    vk::Format swapchain_format = renderer_ctx.get_swapchain().get_format();
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = renderer_ctx.get_instance();
    init_info.PhysicalDevice = renderer_ctx.get_physical_device();
    init_info.Device = renderer_ctx.get_device();
    init_info.QueueFamily = device->get_graphics_family_index();
    init_info.Queue = device->get_graphics_queue();
    init_info.DescriptorPool = imgui_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo =
        vk::PipelineRenderingCreateInfo{}
            .setColorAttachmentFormats(swapchain_format)
            .setDepthAttachmentFormat(
                renderer_ctx.get_swapchain().get_depth_image().get_format());
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);
}

SDL_Window *create_window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(
            std::format("SDL_Init failed: {}", SDL_GetError()));
    }
    SDL_Vulkan_LoadLibrary(nullptr);
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
