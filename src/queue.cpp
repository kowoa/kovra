#include "queue.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Queue::Queue(QueueFamily family, const std::shared_ptr<Device> &device)
    : device{device}, family{family},
      queue{device->get().getQueue(family.get_index(), 0)} {
    spdlog::debug("Queue::Queue()");
}
} // namespace kovra
