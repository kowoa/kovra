#include "frame.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "gpu_data.hpp"

namespace kovra {
Frame::Frame(const Device &device)
    : present_semaphore{device.get().createSemaphoreUnique({})},
      render_semaphore{device.get().createSemaphoreUnique({})},
      render_fence{
          device.get().createFenceUnique({vk::FenceCreateFlagBits::eSignaled})},
      cmd_encoder{std::make_unique<CommandEncoder>(device)},
      desc_allocator{std::make_unique<DescriptorAllocator>(device.get(), 1000)},
      scene_buffer{std::make_unique<GpuBuffer>(device.create_buffer(
          sizeof(GpuSceneData), vk::BufferUsageFlagBits::eUniformBuffer,
          VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT))} {}

void Frame::draw(const DrawContext &ctx) {
    auto device = ctx.device.get()->get();

    // Wait until the GPU has finished rendering the last frame
    if (device.waitForFences(render_fence.get(), vk::True, 1000000000) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for render fence");
    }
    device.resetFences(render_fence.get());

    // Clear descriptor pools
    desc_allocator.get()->clear_pools(device);

    // Create a descriptor set for the scene buffer
    GpuSceneData scene_data{
        GpuCameraData{ctx.camera.viewproj_mat(ctx.swapchain.)}};
}
} // namespace kovra
