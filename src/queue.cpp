#include "queue.hpp"

namespace kovra {
Queue::Queue(vk::Queue queue, std::weak_ptr<Device> device)
    : queue{queue}, device{device} {}
} // namespace kovra
