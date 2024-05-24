#pragma once

#include "kat/config.hpp"

#include <Windows.h>
#include <string>

#include <glm/glm.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <entt/entt.hpp>

#include <optional>

#define KAT_SIGNAL(name, sign) ::entt::sigh<sign> name##Signal; ::entt::sink<decltype(name##Signal)> name{name##Signal}
#define KAT_GLOBAL_SIGNAL(name, sign) inline ::entt::sigh<sign> name##Signal; inline ::entt::sink<decltype(name##Signal)> name{name##Signal}

namespace kat {
    class Window;
    struct WindowSettings;

    inline const wchar_t* WCNAME = L"KatWindowClass";

    struct Version {
        unsigned int major, minor, patch;
    };

    struct GlobalState {
        HINSTANCE hInstance;
        int nCmdShow;
        std::string appName;
        Version appVersion;

        bool started = false;
        bool keepOpen = false; // set this to true to prevent the system from posting a WM_QUIT when the last window is destroyed (ex: if you need to recreate the window for some reason).

        entt::registry entt_registry;

        std::unordered_map<size_t, std::unique_ptr<Window>> windows;
        size_t window_idcounter = 0;

        vk::Instance vkInstance;
        vk::DispatchLoaderDynamic dldy;
        vk::DebugUtilsMessengerEXT vkDebugMessenger;
        vk::PhysicalDevice physicalDevice;
    };

    extern GlobalState *globalState;

    // only called when keepOpen is on and there are no more windows
    KAT_GLOBAL_SIGNAL(OnNoWindowsOpen, void());
    KAT_GLOBAL_SIGNAL(OnNewWindow, void(size_t, const std::unique_ptr<Window>&));


    struct EngineInitInfo {
        HINSTANCE hInstance;

        int nCmdShow = SW_NORMAL;
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

    void destroyWindow(_In_ HWND hwnd);

    void destroyWindow(_In_ size_t id);

    Window* getWindow(_In_ HWND hwnd);

    const std::unique_ptr<Window>& getWindow(_In_ size_t id);

    std::optional<int> pollMessages(_In_ LPMSG pMsg);

    void start();

    size_t windowCount();

    void setKeepOpen(bool keepOpen);
}// namespace kat
