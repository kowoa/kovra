#include "instance.hpp"
#include "SDL_vulkan.h"

namespace kovra {
std::vector<const char *> get_required_instance_extensions(SDL_Window *window);

Instance::Instance(SDL_Window *window) {
    vk::ApplicationInfo app_info{
        "Kovra", VK_MAKE_VERSION(1, 0, 0), "Kovra Engine",
        VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3};
    req_instance_exts = get_required_instance_extensions(window);
};

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

    return exts;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
    // Select prefix depending on flags passed to the callback
    std::string prefix;

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
#if defined(_WIN32)
        prefix = "\033[32m" + prefix + "\033[0m";
#endif
        prefix = "VERBOSE: ";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        prefix = "INFO: ";
#if defined(_WIN32)
        prefix = "\033[36m" + prefix + "\033[0m";
#endif
    } else if (
        messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        prefix = "WARNING: ";
#if defined(_WIN32)
        prefix = "\033[33m" + prefix + "\033[0m";
#endif
    } else if (
        messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        prefix = "ERROR: ";
#if defined(_WIN32)
        prefix = "\033[31m" + prefix + "\033[0m";
#endif
    }

    // Display message to default output (console/logcat)
    std::stringstream debugMessage;
    debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "]["
                 << pCallbackData->pMessageIdName
                 << "] : " << pCallbackData->pMessage;

#if defined(__ANDROID__)
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOGE("%s", debugMessage.str().c_str());
    } else {
        LOGD("%s", debugMessage.str().c_str());
    }
#else
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << debugMessage.str() << "\n\n";
    } else {
        std::cout << debugMessage.str() << "\n\n";
    }
    fflush(stdout);
#endif

    // The return value of this callback controls whether the Vulkan call that
    // caused the validation message will be aborted or not We return VK_FALSE
    // as we DON'T want Vulkan calls that cause a validation message to abort If
    // you instead want to have calls abort, pass in VK_TRUE and the function
    // will return VK_ERROR_VALIDATION_FAILED_EXT
    return VK_FALSE;
}
} // namespace kovra
