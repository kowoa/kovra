#pragma once

#define VULKAN_HPP_EXCEPTIONS

#include "asset-loader.hpp"
#include "context.hpp"
#include "frame.hpp"
#include "image.hpp"

namespace kovra {
// Forward declarations
class RenderResources;

class Renderer
{
  public:
    explicit Renderer(SDL_Window *window);
    ~Renderer();
    Renderer() = delete;
    Renderer(const Renderer &) = delete;

    void draw_frame(Camera &camera);
    void load_gltf(const std::filesystem::path &filepath) noexcept;

    void set_render_scale(float scale) noexcept;

    [[nodiscard]] const Context &get_context() const noexcept
    {
        return *context;
    }
    [[nodiscard]] Context &get_context_mut() const noexcept { return *context; }
    [[nodiscard]] uint32_t get_frame_number() const noexcept
    {
        return frame_number;
    }

  private:
    static constexpr const uint32_t FRAME_OVERLAP = 2;

    std::unique_ptr<Context> context;
    std::unique_ptr<AssetLoader> asset_loader;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;

    // Resources
    std::shared_ptr<RenderResources> render_resources;
    std::unique_ptr<GpuImage> draw_image;

    // ImGui
    VkDescriptorPool imgui_pool;

    float render_scale = 1.0f;

    [[nodiscard]] Frame &get_current_frame() const noexcept
    {
        return *frames.at(frame_number % frames.size());
    }

    void init_imgui(SDL_Window *window);
};
} // namespace kovra
