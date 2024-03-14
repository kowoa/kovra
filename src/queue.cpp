#include "queue.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Queue::Queue(QueueFamily family, const vk::Device &device)
    : family{family}, queue{device.getQueue(family.get_index(), 0)} {
    spdlog::debug("Queue::Queue()");
}
} // namespace kovra
