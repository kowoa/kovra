#include "material.hpp"
#include "mesh.hpp"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "descriptor.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#include "spdlog/spdlog.h"

namespace kovra {
void
init_desc_set_layouts(const vk::Device &device, RenderResources &resources);
void
init_materials(
  const vk::Device &device,
  const Swapchain &swapchain,
  const GpuImage &draw_image,
  RenderResources &resources
);
vk::Sampler
create_sampler(vk::Filter filter, const vk::Device &device);
void
init_default_textures(const Device &device, RenderResources &resources);

Renderer::Renderer(SDL_Window *window)
  : context{ std::make_unique<Context>(window) }
  , frame_number{ 0 }
  , render_resources{
      std::make_shared<RenderResources>(context->get_device_owned())
  }
{
    spdlog::debug("Renderer::Renderer()");

    // Create frames
    frames.reserve(FRAME_OVERLAP);
    for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
        frames.emplace_back(std::make_unique<Frame>(*context->get_device_owned()
        ));
    }

    // Create descriptor set layouts
    init_desc_set_layouts(context->get_device().get(), *render_resources);

    // Create samplers
    render_resources->add_sampler(
      vk::Filter::eNearest,
      create_sampler(vk::Filter::eNearest, context->get_device().get())
    );
    render_resources->add_sampler(
      vk::Filter::eLinear,
      create_sampler(vk::Filter::eLinear, context->get_device().get())
    );

    // Create draw image
    {
        auto swapchain_extent = context->get_swapchain().get_extent();
        draw_image = context->get_device().create_image(GpuImageDescriptor{
          .format = vk::Format::eR16G16B16A16Sfloat,
          .extent =
            vk::Extent3D{ swapchain_extent.width, swapchain_extent.height, 1 },
          .usage = vk::ImageUsageFlagBits::eTransferSrc |
                   vk::ImageUsageFlagBits::eTransferDst |
                   vk::ImageUsageFlagBits::eStorage |
                   vk::ImageUsageFlagBits::eColorAttachment,
          .aspect = vk::ImageAspectFlagBits::eColor,
          .mipmapped = false,
          .sampler = render_resources->get_sampler(vk::Filter::eNearest) });
    }

    // Create materials
    init_materials(
      context->get_device().get(),
      context->get_swapchain(),
      *draw_image,
      *render_resources
    );

    // Create textures
    init_default_textures(context->get_device(), *render_resources);

    init_imgui(window);
}

Renderer::~Renderer()
{
    spdlog::debug("Renderer::~Renderer()");
    // Wait until all frames have finished rendering
    for (const auto &frame : frames) {
        auto render_fence = frame->get_render_fence();
        if (const auto result = context->get_device().get().waitForFences(
              1, &render_fence, VK_TRUE, UINT64_MAX
            );
            result != vk::Result::eSuccess) {
            spdlog::error(
              "Failed to wait for render fence: {}", vk::to_string(result)
            );
        }
    }

    // Destroy ImGui
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(
      get_context().get_device().get(), imgui_pool, nullptr
    );

    draw_image.reset();
    render_resources.reset();

    // Destroy frames
    for (auto &frame : frames) {
        frame.reset();
    }
    frames.clear();

    context.reset();
}

void
Renderer::draw_frame(Camera &camera)
{
    auto draw_ctx = DrawContext{ .device = context->get_device_owned(),
                                 .swapchain = context->get_swapchain_mut(),
                                 .frame_number = frame_number,
                                 .camera = camera,
                                 .render_resources = render_resources,
                                 .draw_image = *draw_image,
                                 .render_scale = render_scale };

    get_current_frame().draw(draw_ctx);

    frame_number++;
}
void
Renderer::load_gltf(const std::filesystem::path &filepath) noexcept
{
    auto mesh_assets = asset_loader->load_gltf_meshes(*this, filepath);
    if (!mesh_assets.has_value()) {
        return;
    }

    for (auto &&asset : mesh_assets.value()) {
        render_resources->add_mesh_asset(std::move(asset));
    }
}

void
Renderer::set_render_scale(float scale) noexcept
{
    render_scale = scale;
}

vk::Sampler
create_sampler(vk::Filter filter, const vk::Device &device)
{
    return device.createSampler(
      vk::SamplerCreateInfo{}
        .setMagFilter(filter)
        .setMinFilter(filter)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
    );
}

void
init_desc_set_layouts(const vk::Device &device, RenderResources &resources)
{
    auto compute_texture = DescriptorSetLayoutBuilder{}
                             .add_binding(
                               0,
                               vk::DescriptorType::eStorageImage,
                               vk::ShaderStageFlagBits::eCompute
                             )
                             .build(device);
    resources.add_desc_set_layout(
      "compute texture", std::move(compute_texture)
    );

    auto scene =
      DescriptorSetLayoutBuilder{}
        .add_binding(
          0,
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
        )
        .build(device);
    resources.add_desc_set_layout("scene", std::move(scene));

    auto texture = DescriptorSetLayoutBuilder{}
                     .add_binding(
                       0,
                       vk::DescriptorType::eCombinedImageSampler,
                       vk::ShaderStageFlagBits::eFragment
                     )
                     .build(device);
    resources.add_desc_set_layout("texture", std::move(texture));
}

void
init_materials(
  const vk::Device &device,
  const Swapchain &swapchain,
  const GpuImage &draw_image,
  RenderResources &resources
)
{
    // Background
    {
        auto desc_set_layouts =
          std::array{ resources.get_desc_set_layout("compute texture") };
        auto pipeline_layout = device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}.setSetLayouts(desc_set_layouts)
        );
        auto background =
          ComputeMaterialBuilder{}
            .set_pipeline_layout(std::move(pipeline_layout))
            .set_shader(std::make_unique<ComputeShader>(ComputeShader{
              "solid-background", device }))
            .build(device);
        resources.add_material("background", std::move(background));
    }

    // Grid
    {
        auto desc_set_layouts =
          std::array{ resources.get_desc_set_layout("scene") };
        auto pipeline_layout = device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}.setSetLayouts(desc_set_layouts)
        );
        auto grid =
          GraphicsMaterialBuilder{}
            .set_pipeline_layout(std::move(pipeline_layout))
            .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{
              "grid", device }))
            .set_color_attachment_format(draw_image.get_format())
            .set_depth_attachment_format(swapchain.get_depth_image().get_format(
            ))
            .build(device);
        resources.add_material("grid", std::move(grid));
    }

    // Mesh
    {
        auto desc_set_layouts =
          std::array{ resources.get_desc_set_layout("scene") };
        auto push_constant_ranges =
          std::array{ vk::PushConstantRange{}
                        .setStageFlags(
                          vk::ShaderStageFlagBits::eVertex |
                          vk::ShaderStageFlagBits::eFragment
                        )
                        .setOffset(0)
                        .setSize(sizeof(GpuPushConstants)) };
        auto pipeline_layout = device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}
            .setSetLayouts(desc_set_layouts)
            .setPushConstantRanges(push_constant_ranges)
        );
        auto mesh =
          GraphicsMaterialBuilder{}
            .set_pipeline_layout(std::move(pipeline_layout))
            .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{
              "mesh-colored", device }))
            .set_color_attachment_format(draw_image.get_format())
            .set_depth_attachment_format(swapchain.get_depth_image().get_format(
            ))
            .build(device);
        resources.add_material("colored mesh", std::move(mesh));
    }

    // Textured mesh
    {
        auto desc_set_layouts =
          std::array{ resources.get_desc_set_layout("scene"),
                      resources.get_desc_set_layout("texture") };
        auto push_constant_ranges =
          std::array{ vk::PushConstantRange{}
                        .setStageFlags(
                          vk::ShaderStageFlagBits::eVertex |
                          vk::ShaderStageFlagBits::eFragment
                        )
                        .setOffset(0)
                        .setSize(sizeof(GpuPushConstants)) };
        auto pipeline_layout = device.createPipelineLayoutUnique(
          vk::PipelineLayoutCreateInfo{}
            .setSetLayouts(desc_set_layouts)
            .setPushConstantRanges(push_constant_ranges)
        );
        auto mesh =
          GraphicsMaterialBuilder{}
            .set_pipeline_layout(std::move(pipeline_layout))
            .set_shader(std::make_unique<GraphicsShader>(GraphicsShader{
              "mesh-textured", device }))
            .set_color_attachment_format(draw_image.get_format())
            .set_depth_attachment_format(swapchain.get_depth_image().get_format(
            ))
            .build(device);
        resources.add_material("textured mesh", std::move(mesh));
    }
}

void
init_default_textures(const Device &device, RenderResources &resources)
{
    uint32_t white = 0xFFFFFFFF;
    resources.add_texture(
      "white",
      device.create_color_image(
        &white,
        1,
        1,
        resources.get_sampler(vk::Filter::eNearest),
        vk::Format::eR8G8B8A8Unorm
      )
    );

    uint32_t gray = 0xFFAAAAAA;
    resources.add_texture(
      "gray",
      device.create_color_image(
        &gray,
        1,
        1,
        resources.get_sampler(vk::Filter::eNearest),
        vk::Format::eR8G8B8A8Unorm
      )
    );

    uint32_t black = 0xFF000000;
    resources.add_texture(
      "black",
      device.create_color_image(
        &black,
        1,
        1,
        resources.get_sampler(vk::Filter::eNearest),
        vk::Format::eR8G8B8A8Unorm
      )
    );

    uint32_t magenta = 0xFFFF00FF;
    std::array<uint32_t, 16 * 16> checkerboard;
    for (size_t i = 0; i < checkerboard.size(); i++) {
        checkerboard[i] = (i % 16 + i / 16) % 2 == 0 ? magenta : black;
    }
    resources.add_texture(
      "checkerboard",
      device.create_color_image(
        &checkerboard,
        16,
        16,
        resources.get_sampler(vk::Filter::eNearest),
        vk::Format::eR8G8B8A8Unorm
      )
    );
}

void
Renderer::init_imgui(SDL_Window *window)
{
    const auto &ctx = get_context();

    const std::shared_ptr<Device> &device{ ctx.get_device_owned() };

    // Create descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(ctx.get_device().get(), &pool_info, nullptr, &imgui_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool for ImGui");
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = ctx.get_instance();
    init_info.PhysicalDevice = ctx.get_physical_device();
    init_info.Device = ctx.get_device().get();
    init_info.QueueFamily = device->get_graphics_family_index();
    init_info.Queue = device->get_graphics_queue();
    init_info.DescriptorPool = imgui_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;
    auto color_formats = std::array{ ctx.get_swapchain().get_format() };
    init_info.PipelineRenderingCreateInfo =
      vk::PipelineRenderingCreateInfo{}
        .setColorAttachmentFormats(color_formats)
        .setDepthAttachmentFormat(
          ctx.get_swapchain().get_depth_image().get_format()
        );
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);
}

} // namespace kovra
