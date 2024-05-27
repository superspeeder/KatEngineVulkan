#pragma once

#include "kat/config.hpp"

#include <Windows.h>
#include <string>

#include <glm/glm.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <entt/entt.hpp>

#include <optional>

#include <glfw/glfw3.h>

#define KAT_SIGNAL(name, sign) ::entt::sigh<sign> name##Signal; ::entt::sink<decltype(name##Signal)> name{name##Signal}
#define KAT_GLOBAL_SIGNAL(name, sign) inline ::entt::sigh<sign> name##Signal; inline ::entt::sink<decltype(name##Signal)> name{name##Signal}

namespace kat {
    class Window;
    struct WindowSettings;

    struct Version {
        unsigned int major, minor, patch;
    };

    struct GlobalState {
        std::string appName;
        Version appVersion;

        bool started = false;
        bool keepOpen = false; // set this to true to prevent the system from posting a WM_QUIT when the last window is destroyed (ex: if you need to recreate the window for some reason).

        entt::registry entt_registry;

        std::unordered_map<size_t, std::unique_ptr<Window>> windows;
        size_t window_idcounter = 0;

        std::optional<int> exitCode = std::nullopt;

        vk::Instance vkInstance;
        vk::DispatchLoaderDynamic dldy;
        vk::DebugUtilsMessengerEXT vkDebugMessenger;
        vk::PhysicalDevice physicalDevice;
        uint32_t primaryQueueFamily, transferQueueFamily; // it's so uncommon that the primary graphics queue *doesn't support present* that we are just going to err if that ever happens.
        vk::Device device;
        vk::Queue primaryQueue, transferQueue;

    };

    extern GlobalState *globalState;

    // only called when keepOpen is on and there are no more windows
    KAT_GLOBAL_SIGNAL(OnNoWindowsOpen, void());
    KAT_GLOBAL_SIGNAL(OnNewWindow, void(size_t, const std::unique_ptr<Window>&));
    KAT_GLOBAL_SIGNAL(OnJoystickReconfigure, void(int, bool));


    struct EngineInitInfo {
        std::string appName = "Application";
        Version appVersion = Version{0, 1, 0};
        bool enableDebug = false;
    };

    void init(_In_ const EngineInitInfo &initInfo);
    void terminate();

    [[nodiscard]] inline entt::registry&entityRegistry() noexcept {
        return globalState->entt_registry;
    }

    [[nodiscard]] inline entt::entity createEntity() {
        return globalState->entt_registry.create();
    }

    // purposeful lack of [[nodiscard]]. allows us to create windows and not care about keeping track of it.
    size_t createWindow(_In_ const WindowSettings& settings);

    void destroyWindow(_In_ GLFWwindow* window);

    void destroyWindow(_In_ size_t id);

    Window* getWindow(_In_ GLFWwindow* window);

    const std::unique_ptr<Window>& getWindow(_In_ size_t id);

    std::optional<int> pollMessages();

    void start();

    size_t windowCount();

    void setKeepOpen(bool keepOpen);

    void updateWindows();
}// namespace kat
