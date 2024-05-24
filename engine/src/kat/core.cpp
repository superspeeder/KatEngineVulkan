#include "core.hpp"
#include <spdlog/spdlog.h>

#include "kat/window.hpp"

namespace kat {

    vk::Instance createVulkanInstance(_In_ const std::string &appName,
                                      _In_ const Version &version,
                                      _Inout_ vk::DispatchLoaderDynamic &dldy,
                                      _In_ bool enableDebug,
                                      _Out_opt_ vk::DebugUtilsMessengerEXT *dbgMsngr);
    vk::PhysicalDevice selectPhysicalDevice();

    bool isPhysicalDeviceSupported(_In_ const vk::PhysicalDevice& pd);
    bool isPhysicalDeviceOptimal(_In_ const vk::PhysicalDevice& pd);

    GlobalState *globalState;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {

        auto type = static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType);
        auto type_str = vk::to_string(type);

        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                spdlog::debug("({}) {}", type_str, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                spdlog::info("({}) {}", type_str, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                spdlog::warn("({}) {}", type_str, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                spdlog::error("({}) {}", type_str, pCallbackData->pMessage);
                break;
            default:
                spdlog::warn("* ({}) {}", type_str, pCallbackData->pMessage);
                break;
        }

        return VK_FALSE;
    }

    void init(_In_ const EngineInitInfo &initInfo) {
        if (!globalState) {
            globalState = new GlobalState();
            globalState->hInstance = initInfo.hInstance;
            globalState->nCmdShow = initInfo.nCmdShow;
            globalState->appName = initInfo.appName;
            globalState->appVersion = initInfo.appVersion;
            globalState->vkInstance = createVulkanInstance(initInfo.appName, initInfo.appVersion, globalState->dldy, initInfo.enableDebug, &globalState->vkDebugMessenger);
            globalState->physicalDevice = selectPhysicalDevice();

            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(wc);
            wc.hInstance = initInfo.hInstance;
            wc.lpszClassName = WCNAME;
            wc.lpfnWndProc = Window::globalProc;
            wc.style = CS_HREDRAW | CS_VREDRAW;

            ATOM a = RegisterClassExW(&wc);
            if (a == 0) {
                throw std::runtime_error("Failed to register window class");
            }

            spdlog::info("Registered Window Class");
        }
    }

    void terminate() {
        if (globalState) {
            if (globalState->vkDebugMessenger) {
                globalState->vkInstance.destroy(globalState->vkDebugMessenger, nullptr, globalState->dldy);
            }

            globalState->vkInstance.destroy();

            delete globalState;
            globalState = nullptr;
        }
    }

    vk::Instance createVulkanInstance(_In_ const std::string &appName,
                                      _In_ const Version &version,
                                      _Inout_ vk::DispatchLoaderDynamic &dldy,
                                      _In_ bool enableDebug,
                                      _Out_opt_ vk::DebugUtilsMessengerEXT *dbgMsngr) {
        vk::ApplicationInfo appInfo{};
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, KATENGINE_VERSION_MAJOR, KATENGINE_VERSION_MINOR, KATENGINE_VERSION_PATCH);
        appInfo.pEngineName = "KatEngine";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, version.major, version.minor, version.patch);
        appInfo.pApplicationName = appName.c_str();

        std::vector<const char *> extensions = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        };

        std::vector<const char *> layers = {};

        vk::DebugUtilsMessengerCreateInfoEXT dci{};
        vk::InstanceCreateInfo ici{};

        if (enableDebug) {
            dci.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
            dci.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            dci.pfnUserCallback = debugCallback;
            ici.pNext = &dci;
            layers.push_back("VK_LAYER_KHRONOS_validation");
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        ici.setPEnabledExtensionNames(extensions).setPEnabledLayerNames(layers);
        ici.pApplicationInfo = &appInfo;

        vk::Instance instance = vk::createInstance(ici);

        dldy = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
        dldy.init(instance);

        if (enableDebug && dbgMsngr) {
            *dbgMsngr = instance.createDebugUtilsMessengerEXT(dci, nullptr, dldy);
        }

        return instance;
    }

    vk::PhysicalDevice selectPhysicalDevice() {
        auto physicalDevices = globalState->vkInstance.enumeratePhysicalDevices();
        vk::PhysicalDevice supportedDevice;
        for (const auto& pd : physicalDevices) {
            if (isPhysicalDeviceSupported(pd)) {
                if (isPhysicalDeviceOptimal(pd)) {
                    vk::PhysicalDeviceProperties props = pd.getProperties();
                    spdlog::debug("Found supported physical device (optimal): {}", props.deviceName.data());
                    return pd;
                }

                supportedDevice = pd;
            }
        }

        if (!supportedDevice) {
            throw std::runtime_error("No supported devices");
        }

        vk::PhysicalDeviceProperties props = supportedDevice.getProperties();
        spdlog::debug("Found supported physical device (non-optimal): {}", props.deviceName.data());
        return supportedDevice;
    }

    size_t createWindow(_In_ const WindowSettings &settings) {
        size_t id = globalState->window_idcounter++;
        globalState->windows[id] = std::make_unique<Window>(settings, id);
        OnNewWindowSignal.publish(id, globalState->windows[id]);
        return id;
    }

    bool isPhysicalDeviceSupported(_In_ const vk::PhysicalDevice &pd) {
        auto queueFamilies = pd.getQueueFamilyProperties();

        uint32_t queueFamilyIndex = 0;

        bool supportsPresentation = false;

        for (const auto& queueProperties : queueFamilies) {
            if (pd.getWin32PresentationSupportKHR(queueFamilyIndex)) { supportsPresentation = true; }

            queueFamilyIndex++;
        }

        if (!supportsPresentation) {
            return false;
        }

        return true;
    }

    bool isPhysicalDeviceOptimal(_In_ const vk::PhysicalDevice &pd) {
        vk::PhysicalDeviceProperties props = pd.getProperties();

        if (props.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
            return false;
        }

        return true;
    }

    void destroyWindow(_In_ HWND hwnd) {
        Window* window = getWindow(hwnd);
        if (window) {
            destroyWindow(window->getId());
        }
    }

    void destroyWindow(_In_ size_t id) {
        auto it = globalState->windows.find(id);
        if (it == globalState->windows.end()) return; // window not present

        spdlog::info("Destroying Window {}", id);

        it->second->cleanup();
        globalState->windows.erase(it);

        if (globalState->started && globalState->windows.empty()) {
            if (globalState->keepOpen) {
                OnNoWindowsOpenSignal.publish();
            } else {
                spdlog::info("Last window destroyed. Exiting.");
                PostQuitMessage(0);
            }
        }
    }

    Window *getWindow(_In_ HWND hwnd) {
        LONG_PTR lp = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (lp) {
            return reinterpret_cast<Window*>(lp);
        }
        return nullptr;
    }

    std::optional<int> pollMessages(LPMSG pMsg) {
        while (PeekMessageW(pMsg, nullptr, 0, 0, PM_REMOVE) != 0) {
            if (pMsg->message == WM_QUIT) {
                return (int)pMsg->wParam;
            }

            TranslateMessage(pMsg);
            DispatchMessageW(pMsg);
        }

        return std::nullopt;
    }

    void start() {
        globalState->started = true;
    }

    const std::unique_ptr<Window> &kat::getWindow(size_t id) {
        return globalState->windows[id];
    }

    size_t windowCount() {
        return globalState->windows.size();
    }

    void setKeepOpen(bool keepOpen) {
        globalState->keepOpen = keepOpen;
    }
}// namespace kat
