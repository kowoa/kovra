#pragma once

#include "queue.hpp"

#include <functional>

namespace kovra {
// Forward declarations
class Device;
class Queue;

class UploadContext {
  public:
    UploadContext(std::shared_ptr<Device> device, Queue queue);
    ~UploadContext();

    // Instantly execute some commands on the GPU without dealing with the
    // render loop and other synchronization.
    // This is great for compute calculates and can be used from a background
    // thread separated from the render loop.
    void immediate_submit(std::function<void(vk::CommandBuffer)> &&function);

  private:
    std::shared_ptr<Device> device;
    vk::UniqueFence upload_fence;
    vk::UniqueCommandPool upload_command_pool;
    vk::UniqueCommandBuffer upload_command_buffer;
    std::unique_ptr<Queue> upload_queue;
};
} // namespace kovra
