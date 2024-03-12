#include "context.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Context::Context(SDL_Window *window) : instance{window} {
    spdlog::info("Context::Context()");
}
} // namespace kovra
