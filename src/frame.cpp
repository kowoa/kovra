#include "frame.hpp"
#include "asset_loader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "cubemap.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "gpu_data.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "pbr_material.hpp"
#include "render_object.hpp"
#include "render_resources.hpp"
#include "swapchain.hpp"
#include "utils.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "spdlog/spdlog.h"

namespace kovra {
Frame::Frame(const Device &device)
  : present_semaphore{ device.get().createSemaphoreUnique({}) }
  , render_semaphore{ device.get().createSemaphoreUnique({}) }
  , render_fence{ device.get().createFenceUnique(
      { vk::FenceCreateFlagBits::eSignaled }
    ) }
  , cmd_encoder{ std::make_unique<CommandEncoder>(device) }
  , desc_allocator{ std::make_unique<DescriptorAllocator>(device.get(), 1000) }
  , scene_buffer{ device.create_buffer(
      sizeof(GpuSceneData),
      vk::BufferUsageFlagBits::eUniformBuffer,
      VMA_MEMORY_USAGE_CPU_TO_GPU,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
  , material_buffer{ device.create_buffer(
      sizeof(GpuPbrMaterialData),
      vk::BufferUsageFlagBits::eUniformBuffer,
      VMA_MEMORY_USAGE_CPU_TO_GPU,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
{
    spdlog::debug("Frame::Frame()");
}

Frame::~Frame()
{
    spdlog::debug("Frame::~Frame()");
    material_buffer.reset();
    scene_buffer.reset();
    desc_allocator.reset();
    cmd_encoder.reset();
    render_fence.reset();
    render_semaphore.reset();
    present_semaphore.reset();
}

void
Frame::draw(const DrawContext &&ctx)
{
    const vk::Device &device = ctx.device.get();

    // Wait until the GPU has finished rendering the last frame (1 sec timeout)
    std::vector<vk::Fence> fences = { render_fence.get() };
    if (device.waitForFences(fences, vk::True, 1000000000) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for render fence");
    }

    // Request image from swapchain (1 sec timeout)
    auto swapchain_image_index = device.acquireNextImageKHR(
      ctx.swapchain.get(), 1000000000, present_semaphore.get(), nullptr
    );
    if (swapchain_image_index.result != vk::Result::eSuccess) {
        switch (swapchain_image_index.result) {
            case vk::Result::eErrorOutOfDateKHR:
                ctx.swapchain.request_resize();
                return; // Early return since failed to acquire swapchain image
            case vk::Result::eSuboptimalKHR:
                ctx.swapchain.request_resize();
                break;
            default:
                spdlog::error("Failed to acquire swapchain image: unknown error"
                );
                throw std::runtime_error("Failed to acquire swapchain image");
        }
    }
    auto swapchain_image =
      ctx.swapchain.get_images().at(swapchain_image_index.value);
    auto swapchain_image_extent = ctx.swapchain.get_extent();

    device.resetFences(render_fence.get());

    // Set draw extent (determines resolution to draw at)
    // This ensures that we don't draw at a higher resolution than the swapchain
    auto draw_extent = ctx.draw_image.get_extent2d();
    draw_extent.setWidth(
      std::min(swapchain_image_extent.width, draw_extent.width) *
      ctx.render_scale
    );
    draw_extent.setHeight(
      std::min(swapchain_image_extent.height, draw_extent.height) *
      ctx.render_scale
    );

    // Clear descriptor pools
    desc_allocator.get()->clear_pools(device);

    // Create a descriptor set for the scene buffer
    auto scene_desc_set_layout =
      ctx.render_resources.get_desc_set_layout("scene");
    auto scene_desc_set =
      desc_allocator.get()->allocate(scene_desc_set_layout, device);

    // Update the scene buffer
    scene_buffer->write(&ctx.scene_data, sizeof(GpuSceneData));

    // Update the scene descriptor set with the updated scene buffer
    auto writer = DescriptorWriter{};
    writer.write_buffer(
      0,
      scene_buffer->get(),
      scene_buffer->get_size(),
      0,
      vk::DescriptorType::eUniformBuffer
    );
    writer.update_set(device, scene_desc_set);

    //--------------------------------------------------------------------------
    cmd_encoder->begin();

    // Transition draw image layout to color attachment optimal for rendering
    cmd_encoder->transition_image_layout(
      ctx.draw_image,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal
    );
    if (ctx.draw_resolve_image != nullptr) {
        cmd_encoder->transition_image_layout(
          *ctx.draw_resolve_image,
          vk::ImageLayout::eUndefined,
          vk::ImageLayout::eColorAttachmentOptimal
        );
    }
    cmd_encoder->transition_image_layout(
      ctx.draw_depth_image,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    // Render to the draw image
    {
        const auto depth_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(ctx.draw_depth_image.get_view())
            .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setDepthStencil({ 1.0f, 0 }));
        auto color_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(ctx.draw_image.get_view())
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setColor({ 0.1f, 0.1f, 0.1f, 1.0f })
            );
        if (ctx.draw_resolve_image != nullptr) {
            color_attachment.setResolveMode(vk::ResolveModeFlagBits::eAverage)
              .setResolveImageView(ctx.draw_resolve_image->get_view())
              .setResolveImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
        }
        const auto render_area =
          vk::Rect2D{}.setOffset({ 0, 0 }).setExtent(draw_extent);
        RenderPass render_pass =
          cmd_encoder->begin_render_pass(RenderPassCreateInfo{
            .color_attachments = { color_attachment },
            .depth_attachment = depth_attachment,
            .render_area = render_area,
          });
        render_pass.set_viewport_scissor(draw_extent.width, draw_extent.height);

        draw_render_objects(render_pass, ctx, scene_desc_set);
        draw_skybox(render_pass, ctx);
        draw_grid(render_pass, ctx, scene_desc_set);
    }

    // Clear swapchain image
    cmd_encoder->transition_image_layout(
      swapchain_image,
      vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal
    );
    cmd_encoder->clear_color_image(
      swapchain_image, vk::ImageLayout::eTransferDstOptimal
    );

    // Resolve multisampled draw image
    if (ctx.draw_resolve_image != nullptr) {
        cmd_encoder->transition_image_layout(
          *ctx.draw_resolve_image,
          vk::ImageLayout::eColorAttachmentOptimal,
          vk::ImageLayout::eTransferSrcOptimal
        );

        // Copy draw image resolve to swapchain image
        cmd_encoder->copy_image_to_image(
          ctx.draw_resolve_image->get(),
          swapchain_image,
          draw_extent,
          swapchain_image_extent
        );
    } else { // Multisampling is disabled
        // Copy draw image to swapchain image
        cmd_encoder->transition_image_layout(
          ctx.draw_image,
          vk::ImageLayout::eColorAttachmentOptimal,
          vk::ImageLayout::eTransferSrcOptimal
        );
        cmd_encoder->copy_image_to_image(
          ctx.draw_image.get(),
          swapchain_image,
          draw_extent,
          swapchain_image_extent
        );
    }

    // ImGui render commands (draw to swapchain image)
    {
        const auto depth_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(ctx.swapchain.get_depth_image().get_view())
            .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setClearValue(vk::ClearValue{}.setDepthStencil({ 1.0f, 0 }));
        const auto color_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(
              ctx.swapchain.get_views().at(swapchain_image_index.value).get()
            )
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setColor({ 0.0f, 0.0f, 0.0f, 1.0f })
            );
        const auto render_area =
          vk::Rect2D{}.setOffset({ 0, 0 }).setExtent(swapchain_image_extent);
        RenderPass render_pass =
          cmd_encoder->begin_render_pass(RenderPassCreateInfo{
            .color_attachments = { color_attachment },
            .depth_attachment = depth_attachment,
            .render_area = render_area,
          });
        render_pass.set_viewport_scissor(
          swapchain_image_extent.width, swapchain_image_extent.height
        );

        // ImGui
        ImGui_ImplVulkan_RenderDrawData(
          ImGui::GetDrawData(), render_pass.get_cmd()
        );
    }

    // Transition swapchain image layout to present src layout
    cmd_encoder->transition_image_layout(
      swapchain_image,
      vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::ePresentSrcKHR
    );

    // Finish recording commands
    auto cmd = cmd_encoder->finish();
    //--------------------------------------------------------------------------

    // Submit command buffer to the graphics queue
    auto wait_stages = std::array<vk::PipelineStageFlags, 1>{
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    };
    ctx.device.get_graphics_queue().submit(
      vk::SubmitInfo{}
        .setPWaitDstStageMask(wait_stages.data())
        .setWaitSemaphores(present_semaphore.get())
        .setSignalSemaphores(render_semaphore.get())
        .setCommandBuffers(cmd),
      render_fence.get()
    );

    present(swapchain_image_index.value, ctx);
}

void
Frame::draw_skybox(RenderPass &pass, const DrawContext &ctx) const
{
    auto skybox_desc_set = desc_allocator->allocate(
      ctx.render_resources.get_desc_set_layout("texture"), ctx.device.get()
    );
    DescriptorWriter writer{};
    writer.write_image(
      0,
      ctx.skybox.get_image().get_view(),
      ctx.render_resources.get_sampler(vk::Filter::eNearest),
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::DescriptorType::eCombinedImageSampler
    );
    writer.update_set(ctx.device.get(), skybox_desc_set);

    pass.set_material(ctx.render_resources.get_material_owned("skybox"));
    std::array<glm::mat4, 2> push_constants = {
        ctx.scene_data.viewproj,
        glm::translate(glm::mat4(1.0f), ctx.camera.get_position())
    };
    pass.set_push_constants(utils::cast_to_bytes(push_constants));
    pass.set_desc_sets(0, { skybox_desc_set }, {});
    pass.draw(36, 1, 0, 0);
}

void
Frame::draw_render_objects(
  RenderPass &pass,
  const DrawContext &ctx,
  const vk::DescriptorSet &scene_desc_set
) const
{
    //--------------------------------------------------------------------------
    ctx.stats.draw_call_count = 0;
    ctx.stats.triangle_count = 0;
    const auto start = std::chrono::system_clock::now();
    //--------------------------------------------------------------------------

    Material *last_material = nullptr;
    MaterialInstance *last_material_instance = nullptr;
    vk::Buffer last_index_buffer = nullptr;

    auto draw_render_object = [&](const RenderObject &object) {
        if (!object.material_instance) {
            spdlog::error("Material Instance is null");
            return;
        }

        // Update material only if different from last material
        if (object.material_instance.get() != last_material_instance) {
            last_material_instance = object.material_instance.get();

            if (object.material_instance->material.get() != last_material) {
                last_material = object.material_instance->material.get();
                pass.set_material(object.material_instance->material);
                pass.set_desc_sets(0, { scene_desc_set });
            }

            pass.set_desc_sets(1, { object.material_instance->desc_set });
        }

        // Update index buffer only if different from last index buffer
        if (object.index_buffer != last_index_buffer) {
            last_index_buffer = object.index_buffer;
            pass.set_index_buffer(object.index_buffer);
        }

        // Update push constants
        pass.set_push_constants(utils::cast_to_bytes(GpuPushConstants{
          .object_transform = object.transform,
          .vertex_buffer = object.vertex_buffer_address }));

        // Draw
        pass.draw_indexed(object.index_count, 1, object.first_index, 0, 0);

        ctx.stats.draw_call_count++;
        ctx.stats.triangle_count += object.index_count / 3;
    };

    std::vector<uint32_t> opaque_draws;
    opaque_draws.reserve(ctx.opaque_objects.size());
    for (size_t i = 0; i < ctx.opaque_objects.size(); i++) {
        opaque_draws.push_back(i);
    }
    // Sort opaque objects by material and mesh.
    // We do this to reduce the number of times the material has to be updated.
    std::sort(
      opaque_draws.begin(),
      opaque_draws.end(),
      [&](const auto &a_idx, const auto &b_idx) {
          const RenderObject &a = ctx.opaque_objects[a_idx];
          const RenderObject &b = ctx.opaque_objects[b_idx];
          // If the material instance is the same, sort by index_buffer
          if (a.material_instance == b.material_instance) {
              return a.index_buffer < b.index_buffer;
          } else {
              // Else compare the material_instance pointer
              return a.material_instance < b.material_instance;
          }
      }
    );

    const auto &viewproj = ctx.scene_data.viewproj;
    for (const auto &object_index : opaque_draws) {
        const auto &object = ctx.opaque_objects[object_index];
        if (object.is_visible(viewproj)) {
            draw_render_object(object);
        }
    }

    for (const auto &object : ctx.transparent_objects) {
        if (object.is_visible(viewproj)) {
            draw_render_object(object);
        }
    }

    //--------------------------------------------------------------------------
    const auto end = std::chrono::system_clock::now();
    const auto elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    ctx.stats.render_objects_draw_time = elapsed.count() / 1000.0f;
    //--------------------------------------------------------------------------
}

void
Frame::draw_grid(
  RenderPass &pass,
  const DrawContext &ctx,
  const vk::DescriptorSet &scene_desc_set
) const
{
    pass.set_material(ctx.render_resources.get_material_owned("grid"));
    pass.set_desc_sets(0, { scene_desc_set }, {});
    pass.draw(6, 1, 0, 0);
}

void
Frame::present(uint32_t swapchain_image_index, const DrawContext &ctx) const
{
    const auto swapchains = std::array{ ctx.swapchain.get() };
    const auto wait_semaphores = std::array{ render_semaphore.get() };
    const auto result = ctx.device.get_present_queue().presentKHR(
      vk::PresentInfoKHR{}
        .setSwapchains(swapchains)
        .setWaitSemaphores(wait_semaphores)
        .setPImageIndices(&swapchain_image_index)
    );
    if (result != vk::Result::eSuccess) {
        switch (result) {
            case vk::Result::eErrorOutOfDateKHR:
                ctx.swapchain.request_resize();
                break;
            case vk::Result::eSuboptimalKHR:
                ctx.swapchain.request_resize();
                break;
            default:
                spdlog::error("Failed to present swapchain image: unknown error"
                );
                throw std::runtime_error("Failed to present swapchain image");
        }
    }
}

} // namespace kovra
