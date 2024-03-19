#include "SDL_vulkan.h"
#include <iostream>
#include <sstream>

#include "instance.hpp"
#include "physical_device.hpp"
#include "spdlog/spdlog.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace kovra {

std::vector<const char *> get_required_instance_extensions(SDL_Window *window);
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

Instance::Instance(SDL_Window *window) : physical_devices{} {
    spdlog::debug("Instance::Instance()");

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

    debug_messenger = instance->createDebugUtilsMessengerEXTUnique(
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

Instance::~Instance() { spdlog::debug("Instance::~Instance()"); }

const std::vector<std::shared_ptr<PhysicalDevice>> &
Instance::enumerate_physical_devices(const Surface &surface) {
    if (physical_devices.empty()) {
        // Enumerate and populate physical_devices
        std::vector<vk::PhysicalDevice> vk_physical_devices =
            instance->enumeratePhysicalDevices();
        for (vk::PhysicalDevice physical_device : vk_physical_devices) {
            physical_devices.emplace_back(
                std::make_shared<PhysicalDevice>(physical_device, surface));
        }
        // Sort so that discrete GPUs come first in the vector
        std::sort(
            physical_devices.begin(), physical_devices.end(),
            [](const std::shared_ptr<PhysicalDevice> &a,
               const std::shared_ptr<PhysicalDevice> &b) {
                vk::PhysicalDeviceType a_type =
                    (*a).get().getProperties().deviceType;
                vk::PhysicalDeviceType b_type =
                    (*b).get().getProperties().deviceType;
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
    std::string type_prefix;
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        type_prefix = "GENERAL";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        type_prefix = "VALIDATION";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        type_prefix = "PERFORMANCE";
    }

    std::string severity_prefix;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        severity_prefix = "VERBOSE: ";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        severity_prefix = "INFO: ";
    } else if (
        messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        severity_prefix = "WARNING: ";
    } else if (
        messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        severity_prefix = "ERROR: ";
    }

    std::stringstream debug_message;
    debug_message << type_prefix << " - " << severity_prefix << " ["
                  << pCallbackData->messageIdNumber << "]["
                  << pCallbackData->pMessageIdName
                  << "] : " << pCallbackData->pMessage;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::error(debug_message.str());
    } else {
        spdlog::info(debug_message.str());
    }

    // The return value of this callback controls whether the Vulkan call that
    // caused the validation message will be aborted or not We return VK_FALSE
    // as we DON'T want Vulkan calls that cause a validation message to abort If
    // you instead want to have calls abort, pass in VK_TRUE and the function
    // will return VK_ERROR_VALIDATION_FAILED_EXT
    return VK_FALSE;
}
#pragma clang diagnostic pop
} // namespace kovra
