#include "SDL_vulkan.h"
#include <iostream>
#include <sstream>

#include "instance.hpp"
#include "physical_device.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace kovra {

std::vector<const char *> get_required_instance_extensions(SDL_Window *window);
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

Instance::Instance(SDL_Window *window) : physical_devices{} {
    std::cout << "Instance::Instance()" << std::endl;

    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    std::vector<const char *> req_instance_exts =
        get_required_instance_extensions(window);
    std::vector<const char *> req_validation_layers = {
        "VK_LAYER_KHRONOS_validation"};

    vk::ApplicationInfo app_info(
        "Kovra", VK_MAKE_VERSION(1, 0, 0), "Kovra Engine",
        VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);

    instance = vk::createInstanceUnique(
        vk::InstanceCreateInfo{
            {},
            &app_info,
            static_cast<uint32_t>(req_validation_layers.size()),
            req_validation_layers.data(),
            static_cast<uint32_t>(req_instance_exts.size()),
            req_instance_exts.data()},
        nullptr);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

    debug_utils = instance->createDebugUtilsMessengerEXTUnique(
        vk::DebugUtilsMessengerCreateInfoEXT{
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debugUtilsMessageCallback},
        nullptr);
}

const std::vector<PhysicalDevice> &
Instance::enumerate_physical_devices(const Surface &surface) {
    if (physical_devices.empty()) {
        // Enumerate and populate physical_devices
        std::vector<vk::PhysicalDevice> vk_physical_devices =
            instance->enumeratePhysicalDevices();
        for (vk::PhysicalDevice physical_device : vk_physical_devices) {
            physical_devices.emplace_back(physical_device, *this, surface);
        }
        // Sort so that discrete GPUs come first in the vector
        std::sort(
            physical_devices.begin(), physical_devices.end(),
            [](const PhysicalDevice &a, const PhysicalDevice &b) {
                vk::PhysicalDeviceType a_type =
                    a.get().getProperties().deviceType;
                vk::PhysicalDeviceType b_type =
                    b.get().getProperties().deviceType;
                int a_score =
                    (a_type == vk::PhysicalDeviceType::eDiscreteGpu)     ? 0
                    : (a_type == vk::PhysicalDeviceType::eIntegratedGpu) ? 1
                                                                         : 2;
                int b_score =
                    (b_type == vk::PhysicalDeviceType::eDiscreteGpu)     ? 0
                    : (b_type == vk::PhysicalDeviceType::eIntegratedGpu) ? 1
                                                                         : 2;
                return a_score < b_score;
            });
    }
    return physical_devices;
}

std::vector<const char *> get_required_instance_extensions(SDL_Window *window) {
    uint32_t ext_count;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr)) {
        throw std::runtime_error(std::format(
            "SDL_Vulkan_GetInstanceExtensions failed: {}", SDL_GetError()));
    }

    std::vector<const char *> exts(ext_count);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, exts.data())) {
        throw std::runtime_error(std::format(
            "SDL_Vulkan_GetInstanceExtensions failed: {}", SDL_GetError()));
    }

    // For validation layers
    exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return exts;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
    // Select prefix depending on flags passed to the callback
    std::string prefix;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        prefix = "VERBOSE: ";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        prefix = "INFO: ";
    } else if (
        messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        prefix = "WARNING: ";
    } else if (
        messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        prefix = "ERROR: ";
    }

    // Display message to default output
    std::stringstream debugMessage;
    debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "]["
                 << pCallbackData->pMessageIdName
                 << "] : " << pCallbackData->pMessage;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << debugMessage.str() << "\n\n";
    } else {
        std::cout << debugMessage.str() << "\n\n";
    }
    fflush(stdout);

    // The return value of this callback controls whether the Vulkan call that
    // caused the validation message will be aborted or not We return VK_FALSE
    // as we DON'T want Vulkan calls that cause a validation message to abort If
    // you instead want to have calls abort, pass in VK_TRUE and the function
    // will return VK_ERROR_VALIDATION_FAILED_EXT
    return VK_FALSE;
}
#pragma clang diagnostic pop
} // namespace kovra
