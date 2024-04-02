#include "frame.hpp"
#include "asset_loader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
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
Frame::draw(const DrawContext &ctx)
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
      ctx.draw_image.get(),
      vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal
    );

    // Render commands (draw to draw image)
    {
        auto color_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(ctx.draw_image.get_view())
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setColor({ 0.1f, 0.1f, 0.1f, 1.0f })
            );
        auto depth_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(ctx.swapchain.get_depth_image().get_view())
            .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setDepthStencil({ 1.0f, 0 }));
        auto render_area =
          vk::Rect2D{}.setOffset({ 0, 0 }).setExtent(draw_extent);
        RenderPass render_pass =
          cmd_encoder->begin_render_pass(RenderPassDescriptor{
            .color_attachments = { color_attachment },
            .depth_attachment = depth_attachment,
            .render_area = render_area,
          });

        render_pass.set_viewport_scissor(draw_extent.width, draw_extent.height);

        draw_meshes(render_pass, ctx, scene_desc_set);
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

    // Copy draw image to swapchain image
    cmd_encoder->transition_image_layout(
      ctx.draw_image.get(),
      vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::eTransferSrcOptimal
    );
    cmd_encoder->copy_image_to_image(
      ctx.draw_image.get(), swapchain_image, draw_extent, swapchain_image_extent
    );

    // ImGui render commands (draw to swapchain image)
    {
        auto color_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(
              ctx.swapchain.get_views().at(swapchain_image_index.value).get()
            )
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setColor({ 0.0f, 0.0f, 0.0f, 1.0f })
            );
        auto depth_attachment =
          vk::RenderingAttachmentInfo{}
            .setImageView(ctx.swapchain.get_depth_image().get_view())
            .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setDepthStencil({ 1.0f, 0 }));
        auto render_area =
          vk::Rect2D{}.setOffset({ 0, 0 }).setExtent(swapchain_image_extent);
        RenderPass render_pass =
          cmd_encoder->begin_render_pass(RenderPassDescriptor{
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
Frame::draw_meshes(
  RenderPass &pass,
  const DrawContext &ctx,
  const vk::DescriptorSet &scene_desc_set
)
{
    /*
      auto texture_desc_set = desc_allocator->allocate(
        ctx.render_resources.get_desc_set_layout("texture"), ctx.device.get()
      );
      {
          const GpuImage &texture =
            ctx.render_resources.get_texture("checkerboard");
          DescriptorWriter writer{};
          writer.write_image(
            0,
            texture.get_view(),
            texture.get_sampler(),
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::DescriptorType::eCombinedImageSampler
          );
          writer.update_set(ctx.device.get(), texture_desc_set);
      }

      pass.set_material(ctx.render_resources.get_material_owned("textured
      mesh")); pass.set_desc_sets(0, { scene_desc_set, texture_desc_set }, {});
      for (const auto &mesh_asset : ctx.render_resources.get_mesh_assets()) {
          if (mesh_asset->mesh->get_id() != 2) {
              continue;
          }

          const Mesh &mesh = *mesh_asset->mesh;
          pass.set_push_constants(utils::cast_to_bytes(GpuPushConstants{
            .vertex_buffer = mesh.get_vertex_buffer_address() }));
          pass.set_index_buffer(mesh.get_index_buffer().get());
          for (const auto &surface : mesh_asset->surfaces) {
              pass.draw_indexed(surface.count, 1, surface.start_index, 0, 0);
          }
      }
    */

    // Draw all opaque render objects
    for (const RenderObject &object : ctx.opaque_objects) {
        pass.set_material(object.material_instance.material);
        pass.set_desc_sets(
          0, { scene_desc_set, object.material_instance.desc_set }, {}
        );
        pass.set_index_buffer(object.index_buffer);
        pass.set_push_constants(utils::cast_to_bytes(GpuPushConstants{
          .vertex_buffer = object.vertex_buffer_address,
          .object_transform = object.transform }));
        pass.draw_indexed(object.index_count, 1, object.first_index, 0, 0);
    }
}

void
Frame::draw_grid(
  RenderPass &pass,
  const DrawContext &ctx,
  const vk::DescriptorSet &scene_desc_set
)
{
    pass.set_material(ctx.render_resources.get_material_owned("grid"));
    pass.set_desc_sets(0, { scene_desc_set }, {});
    pass.draw(6, 1, 0, 0);
}

void
Frame::present(uint32_t swapchain_image_index, const DrawContext &ctx)
{
    auto swapchains = std::array{ ctx.swapchain.get() };
    auto wait_semaphores = std::array{ render_semaphore.get() };
    auto result = ctx.device.get_present_queue().presentKHR(
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
