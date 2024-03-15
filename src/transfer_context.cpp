#include "queue.hpp"
#include "spdlog/spdlog.h"
#include "transfer_context.hpp"

namespace kovra {
TransferContext::TransferContext(Queue queue, const vk::Device &device)
    : transfer_fence{device.createFenceUnique(vk::FenceCreateInfo{})},
      transfer_command_pool{device.createCommandPoolUnique(
          vk::CommandPoolCreateInfo{}
              .setFlags(
                  vk::CommandPoolCreateFlagBits::eTransient |
                  vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
              .setQueueFamilyIndex(queue.get_family_index()))},
      transfer_command_buffer{std::move(device.allocateCommandBuffersUnique(
          vk::CommandBufferAllocateInfo{}
              .setCommandPool(transfer_command_pool.get())
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(1))[0])},
      transfer_queue{std::make_unique<Queue>(queue)} {
    spdlog::debug("TransferContext::TransferContext()");
}

TransferContext::~TransferContext() {
    spdlog::debug("TransferContext::~TransferContext()");
    transfer_queue.reset();
    transfer_command_buffer.reset();
    transfer_command_pool.reset();
    transfer_fence.reset();
}

void TransferContext::immediate_submit(
    std::function<void(vk::CommandBuffer)> &&function,
    const vk::Device &device) {
    vk::CommandBuffer cmd = transfer_command_buffer.get();

    // Record the command buffer
    cmd.begin(vk::CommandBufferBeginInfo{
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    function(cmd);
    cmd.end();

    // Submit the command buffer to the queue
    // upload_fence blocks until execution finished
    transfer_queue->get().submit(
        vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(&cmd),
        transfer_fence.get());

    // Wait for the queue to finish execution
    vk::resultCheck(
        device.waitForFences(transfer_fence.get(), VK_TRUE, UINT64_MAX),
        "Failed to wait for upload fence");

    // Clean up
    device.resetFences(transfer_fence.get());
    device.resetCommandPool(transfer_command_pool.get());
}

} // namespace kovra
