#include "app.hpp"
#include "spdlog/spdlog.h"

#include <vulkan/vulkan.hpp>

int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        auto app = kovra::App{};
        app.run();
    } catch (const vk::SystemError &e) {
        spdlog::error("vk::SystemError: {}", e.what());
        return 1;
    } catch (const std::exception &e) {
        spdlog::error("std::exception: {}", e.what());
        return 1;
    } catch (...) {
        spdlog::error("unknown error");
        return 1;
    }
    return 0;
}
