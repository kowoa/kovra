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
    std::function<void(vk::CommandBuffer)> &&function) {}
} // namespace kovra
