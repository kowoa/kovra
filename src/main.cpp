#include "app.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <vulkan/vulkan.hpp>

int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        auto app = kovra::App{};
        app.run();
    } catch (vk::SystemError &e) {
        std::cout << "vk::SystemError: " << e.what() << std::endl;
        return 1;
    } catch (std::exception &e) {
        std::cout << "std::exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
    return 0;
}
