#include "context.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Context::Context(SDL_Window *window)
    : instance{window}, surface{instance, window} {
    spdlog::info("Context::Context()");
    auto physical_devices = instance.enumerate_physical_devices(surface);
    physical_device = pick_physical_device(physical_devices, surface);
}
} // namespace kovra
