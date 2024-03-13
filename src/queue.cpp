#include "queue.hpp"

namespace kovra {
Queue::Queue(vk::Queue queue, const std::shared_ptr<Device> &device)
    : queue{queue}, device{device} {}
} // namespace kovra
