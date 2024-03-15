#include "frame.hpp"
#include "device.hpp"

namespace kovra {
Frame::Frame(const Device &device) : desc_allocator{device.get(), 1000} {
    vk::SemaphoreCreateInfo semaphore_info;
    vk::FenceCreateInfo fence_info;
    fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

    present_semaphore = device.get().createSemaphoreUnique(semaphore_info);
    render_semaphore = device.get().createSemaphoreUnique(semaphore_info);
    render_fence = device.get().createFenceUnique(fence_info);
}
} // namespace kovra
