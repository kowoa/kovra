#include "upload_context.hpp"
#include "queue.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
UploadContext::UploadContext(Queue queue, const vk::Device &device)
    : upload_fence{device.createFenceUnique(vk::FenceCreateInfo{})},
      upload_command_pool{device.createCommandPoolUnique(
          vk::CommandPoolCreateInfo{}
              .setFlags(
                  vk::CommandPoolCreateFlagBits::eTransient |
                  vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
              .setQueueFamilyIndex(queue.get_family_index()))},
      upload_command_buffer{std::move(device.allocateCommandBuffersUnique(
          vk::CommandBufferAllocateInfo{}
              .setCommandPool(upload_command_pool.get())
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(1))[0])},
      upload_queue{std::make_unique<Queue>(queue)} {
    spdlog::debug("UploadContext::UploadContext()");
}

UploadContext::~UploadContext() {
    spdlog::debug("UploadContext::~UploadContext()");
    upload_queue.reset();
    upload_command_buffer.reset();
    upload_command_pool.reset();
    upload_fence.reset();
}

void UploadContext::immediate_submit(
    std::function<void(vk::CommandBuffer)> &&function,
    const vk::Device &device) {
    vk::CommandBuffer cmd = upload_command_buffer.get();

    // Record the command buffer
    cmd.begin(vk::CommandBufferBeginInfo{
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    function(cmd);
    cmd.end();

    // Submit the command buffer to the queue
    // upload_fence blocks until execution finished
    upload_queue->get().submit(
        vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(&cmd),
        upload_fence.get());

    // Wait for the queue to finish execution
    if (device.waitForFences(upload_fence.get(), VK_TRUE, UINT64_MAX) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("Waiting for upload fence took too long");
    }

    // Clean up
    device.resetFences(upload_fence.get());
    device.resetCommandPool(upload_command_pool.get());
}

} // namespace kovra
