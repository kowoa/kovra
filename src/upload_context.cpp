#include "upload_context.hpp"
#include "device.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
UploadContext::UploadContext(std::shared_ptr<Device> device, Queue queue)
    : device{device},
      upload_fence{device->get().createFenceUnique(vk::FenceCreateInfo{})},
      upload_command_pool{device->get().createCommandPoolUnique(
          vk::CommandPoolCreateInfo{}
              .setFlags(
                  vk::CommandPoolCreateFlagBits::eTransient |
                  vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
              .setQueueFamilyIndex(queue.get_family_index()))},
      upload_command_buffer{
          std::move(device->get().allocateCommandBuffersUnique(
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
    device.reset();
}

void UploadContext::immediate_submit(
    std::function<void(vk::CommandBuffer)> &&function) {}
} // namespace kovra
