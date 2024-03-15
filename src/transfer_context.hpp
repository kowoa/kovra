#pragma once

#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Device;
class Queue;

class TransferContext {
  public:
    TransferContext(Queue queue, const vk::Device &device);
    ~TransferContext();

    // Instantly execute some commands on the GPU without dealing with the
    // render loop and other synchronization.
    // This is great for compute calculates and can be used from a background
    // thread separated from the render loop.
    void immediate_submit(
        std::function<void(vk::CommandBuffer)> &&function,
        const vk::Device &device);

  private:
    vk::UniqueFence transfer_fence;
    vk::UniqueCommandPool transfer_command_pool;
    vk::UniqueCommandBuffer transfer_command_buffer;
    std::unique_ptr<Queue> transfer_queue;
};
} // namespace kovra
