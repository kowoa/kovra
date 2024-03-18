#include "frame.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "gpu_data.hpp"
#include "spdlog/spdlog.h"
#include "swapchain.hpp"

namespace kovra {
Frame::Frame(const Device &device)
    : present_semaphore{device.get().createSemaphoreUnique({})},
      render_semaphore{device.get().createSemaphoreUnique({})},
      render_fence{
          device.get().createFenceUnique({vk::FenceCreateFlagBits::eSignaled})},
      cmd_encoder{std::make_unique<CommandEncoder>(device)},
      desc_allocator{
          std::make_unique<DescriptorAllocator>(device.get(), 1000)} {
    auto buffer = device.create_buffer(
        sizeof(GpuSceneData), vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT);
    scene_buffer = std::make_unique<GpuBuffer>(std::move(buffer));
    spdlog::debug("Frame::Frame()");
}

Frame::~Frame() { spdlog::debug("Frame::~Frame()"); }

void Frame::draw(const DrawContext &ctx) {
    auto device = ctx.device.get()->get();

    // Wait until the GPU has finished rendering the last frame (1 sec timeout)
    std::vector<vk::Fence> fences = {render_fence.get()};
    if (device.waitForFences(fences, vk::True, 1000000000) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for render fence");
    }
    device.resetFences(render_fence.get());

    // Clear descriptor pools
    desc_allocator.get()->clear_pools(device);

    // Create a descriptor set for the scene buffer
    auto scene_desc_set_layout = ctx.desc_set_layouts.at("scene").get();
    auto scene_desc_set =
        desc_allocator.get()->allocate(scene_desc_set_layout, device);

    // Update the scene buffer
    auto swapchain_extent = ctx.swapchain->get_extent();
    GpuSceneData scene_data{
        .camera =
            GpuCameraData{
                .viewproj = ctx.camera.get_viewproj_mat(
                    swapchain_extent.width, swapchain_extent.height),
                .near = ctx.camera.get_near(),
                .far = ctx.camera.get_far(),
            },
        .ambient_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        .sunlight_direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f),
        .sunlight_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    };
    scene_buffer->write(&scene_data, sizeof(GpuSceneData));

    // Update the scene descriptor set with the updated scene buffer
    auto writer = DescriptorWriter{};
    writer.write_buffer(
        0, scene_buffer->get(), scene_buffer->get_size(), 0,
        vk::DescriptorType::eUniformBuffer);
    writer.update_set(device, scene_desc_set.get());

    // Request image from swapchain (1 sec timeout)
    auto swapchain_image_index = device.acquireNextImageKHR(
        ctx.swapchain->get(), 1000000000, present_semaphore.get(), nullptr);
    if (swapchain_image_index.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    auto swapchain_image =
        ctx.swapchain->get_images().at(swapchain_image_index.value);

    draw_background(ctx);
}

void Frame::draw_background(const DrawContext &ctx) {
    ComputePass pass = cmd_encoder->begin_compute_pass();
    // auto background_image = ctx.background_image;
}
} // namespace kovra
