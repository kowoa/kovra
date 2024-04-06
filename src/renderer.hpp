#pragma once

#define VULKAN_HPP_EXCEPTIONS

#include "asset_loader.hpp"
#include "context.hpp"
#include "frame.hpp"
#include "image.hpp"
#include "profiling.hpp"
#include "render_object.hpp"

namespace kovra {
// Forward declarations
class RenderResources;
class PbrMaterial;
class Cubemap;

class Renderer
{
  public:
    explicit Renderer(SDL_Window *window);
    ~Renderer();
    Renderer() = delete;
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer &operator=(Renderer &&) = delete;

    void draw_frame(
      const Camera &camera,
      const std::span<std::string> &objects_to_render
    );

    void load_gltf(
      const std::filesystem::path &filepath,
      const std::string &name
    ) noexcept;
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
    [[nodiscard]] const RenderResources &get_render_resources() const noexcept
    {
        return *render_resources;
    }
    [[nodiscard]] const GpuImage &get_draw_image() const noexcept
    {
        return *draw_image;
    }
    [[nodiscard]] const RendererStats &get_stats() const noexcept
    {
        return stats;
    }

  private:
    std::unique_ptr<Context> context;

    // WARNING: Do NOT clear_pools() this for entire lifetime of the Renderer
    std::unique_ptr<DescriptorAllocator> global_desc_allocator;

    // Frames
    static constexpr const uint32_t FRAME_OVERLAP = 2;
    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;

    // Resources
    std::shared_ptr<RenderResources> render_resources;
    std::unique_ptr<GpuImage> draw_image;
    std::unique_ptr<Cubemap> skybox;

    // ImGui
    VkDescriptorPool imgui_pool;

    float render_scale = 1.0f;

    // Profiling
    RendererStats stats;

    [[nodiscard]] Frame &get_current_frame() const noexcept
    {
        return *frames.at(frame_number % frames.size());
    }

    void init_imgui(SDL_Window *window);

    auto update_scene(
      const Camera &camera,
      const std::span<std::string> &objects_to_render
    ) -> DrawContext;
};
} // namespace kovra
