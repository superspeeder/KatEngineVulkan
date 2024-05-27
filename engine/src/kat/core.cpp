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
    void selectQueueFamilies();
    vk::Device createDevice();
    void gatherQueues();

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
                spdlog::debug("[validation] ({}) {}", type_str, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                spdlog::info("[validation] ({}) {}", type_str, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                spdlog::warn("[validation] ({}) {}", type_str, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                spdlog::error("[validation] ({}) {}", type_str, pCallbackData->pMessage);
                break;
            default:
                spdlog::warn("[validation] * ({}) {}", type_str, pCallbackData->pMessage);
                break;
        }

        return VK_FALSE;
    }

    void init(_In_ const EngineInitInfo &initInfo) {
        if (!globalState) {
            glfwInit();
            glfwSetErrorCallback(+[](int ec, const char* desc) {
                spdlog::error("[glfw] ({}) {}", ec, desc);
            });

            glfwSetJoystickCallback(+[](int jid, int event) {
                OnJoystickReconfigureSignal.publish(jid, event == GLFW_CONNECTED);
            });

            globalState = new GlobalState();
            globalState->appName = initInfo.appName;
            globalState->appVersion = initInfo.appVersion;
            globalState->vkInstance = createVulkanInstance(initInfo.appName, initInfo.appVersion, globalState->dldy, initInfo.enableDebug, &globalState->vkDebugMessenger);
            globalState->physicalDevice = selectPhysicalDevice();
            selectQueueFamilies();
            globalState->device = createDevice();
            gatherQueues();
        }
    }

    void terminate() {
        if (globalState) {
            if (globalState->device) {
                globalState->device.destroy();
            }

            if (globalState->vkDebugMessenger) {
                globalState->vkInstance.destroy(globalState->vkDebugMessenger, nullptr, globalState->dldy);
            }

            globalState->vkInstance.destroy();

            delete globalState;
            globalState = nullptr;

            glfwTerminate();
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

    void destroyWindow(_In_ GLFWwindow* wnd) {
        Window* window = getWindow(wnd);
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

    Window *getWindow(_In_ GLFWwindow* wnd) {
        void* pWindow = glfwGetWindowUserPointer(wnd);
        if (pWindow) {
            return reinterpret_cast<Window*>(pWindow);
        }
        return nullptr;
    }

    std::optional<int> pollMessages() {
        glfwPollEvents();

        if (globalState->exitCode != std::nullopt) return globalState->exitCode;

        if (globalState->windows.empty()) return EXIT_SUCCESS;

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

    void updateWindows() {
        std::vector<size_t> wins_to_destroy;
        for (const auto& win : globalState->windows) {
            if (win.second->shouldClose()) {
                wins_to_destroy.push_back(win.first);
            } else {
                win.second->redraw();
            }
        }

        for (const auto& win : wins_to_destroy) {
            destroyWindow(win);
        }
    }

    bool isPrimaryQueue(uint32_t index, const vk::QueueFamilyProperties& props) {
        return (props.queueFlags & vk::QueueFlagBits::eGraphics) && globalState->physicalDevice.getWin32PresentationSupportKHR(index);
    }

    bool isIdealExplicitTransfer(const vk::QueueFamilyProperties& props) {
        // todo: update my vulkan sdk because I apparently don't have the definitions for VK_KHR_video_encode_queue
        return !(props.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eVideoDecodeKHR | vk::QueueFlagBits::eVideoEncodeKHR)) && (props.queueFlags & (vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eSparseBinding));
    }

    bool isExplicitTransfer(const vk::QueueFamilyProperties& props) {
        return !(props.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eVideoDecodeKHR | vk::QueueFlagBits::eVideoEncodeKHR)) && (props.queueFlags & vk::QueueFlagBits::eTransfer);
    }

    bool isMostlyIdealExplicitTransfer(const vk::QueueFamilyProperties& props) {
        return !(props.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) && (props.queueFlags & (vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eSparseBinding));
    }

    bool isMostlyExplicitTransfer(const vk::QueueFamilyProperties& props) {
        return !(props.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) && (props.queueFlags & vk::QueueFlagBits::eTransfer);
    }

    bool isIdealNonGraphicsTransfer(const vk::QueueFamilyProperties& props) {
        return !(props.queueFlags & vk::QueueFlagBits::eGraphics) && (props.queueFlags & (vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eSparseBinding));
    }

    // by the time we are using this one, we really only want a transfer queue that isn't graphics :(
    bool isNonGraphicsTransfer(const vk::QueueFamilyProperties& props) {
        return !(props.queueFlags & vk::QueueFlagBits::eGraphics) && (props.queueFlags & vk::QueueFlagBits::eTransfer);
    }

    void selectQueueFamilies() {
        uint32_t primaryQueue = UINT32_MAX;
        // prefer sparse binding over explicit transfer queue
        uint32_t idealExplicitTransfer = UINT32_MAX;
        uint32_t mostlyIdealExplicitTransfer = UINT32_MAX;
        uint32_t idealNonGraphicsTransfer = UINT32_MAX;

        uint32_t explicitTransfer = UINT32_MAX;
        uint32_t mostlyExplicitTransfer = UINT32_MAX;
        uint32_t nonGraphicsTransfer = UINT32_MAX;

        auto queueFamilies = globalState->physicalDevice.getQueueFamilyProperties();

        uint32_t index = 0;
        for (const auto& qf : queueFamilies) {
            if (primaryQueue == UINT32_MAX && isPrimaryQueue(index, qf)) {
                primaryQueue = index;
            }

            if (idealExplicitTransfer == UINT32_MAX && isIdealExplicitTransfer(qf)) {
                idealExplicitTransfer = index;
            }

            if (isExplicitTransfer(qf)) {
                explicitTransfer = index;
            }

            if (isMostlyIdealExplicitTransfer(qf)) {
                mostlyIdealExplicitTransfer = index;
            }

            if (isMostlyExplicitTransfer(qf)) {
                mostlyExplicitTransfer = index;
            }

            if (isIdealNonGraphicsTransfer(qf)) {
                idealNonGraphicsTransfer = index;
            }

            if (isNonGraphicsTransfer(qf)) {
                nonGraphicsTransfer = index;
            }

            if (primaryQueue != UINT32_MAX && idealExplicitTransfer != UINT32_MAX) {
                break;
            }

            index++;
        }

        if (primaryQueue == UINT32_MAX) {
            throw std::runtime_error("Missing primary queue");
        }

        if (idealExplicitTransfer != UINT32_MAX) {
            spdlog::debug("Ideal explicit transfer family is found");
            globalState->transferQueueFamily = idealExplicitTransfer;
        } else if (mostlyIdealExplicitTransfer != UINT32_MAX) {
            spdlog::debug("Mostly ideal explicit transfer family is found");
            globalState->transferQueueFamily = mostlyIdealExplicitTransfer;
        } else if (idealNonGraphicsTransfer != UINT32_MAX) {
            spdlog::debug("Ideal non graphics transfer family is found");
            globalState->transferQueueFamily = idealNonGraphicsTransfer;
        } else if (explicitTransfer != UINT32_MAX) {
            spdlog::debug("Explicit transfer family is found");
            globalState->transferQueueFamily = explicitTransfer;
        } else if (mostlyExplicitTransfer != UINT32_MAX) {
            spdlog::debug("Mostly explicit transfer family is found");
            globalState->transferQueueFamily = mostlyExplicitTransfer;
        } else if (nonGraphicsTransfer != UINT32_MAX) {
            spdlog::debug("Non graphics transfer family is found");
            globalState->transferQueueFamily = nonGraphicsTransfer;
        } else {
            spdlog::debug("Transfer family falling back on primary");
            globalState->transferQueueFamily = primaryQueue; // no extra transfer queue which isn't graphics
        }

        globalState->primaryQueueFamily = primaryQueue;
    }

    vk::Device createDevice() {
        vk::DeviceCreateInfo ci{};

        std::vector<const char*> extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::array<float, 1> priorities = { 1.0f };

        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, globalState->primaryQueueFamily, priorities);
        if (globalState->primaryQueueFamily != globalState->transferQueueFamily) {
            // exclusive queue found
            queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, globalState->transferQueueFamily, priorities);
        }

        vk::PhysicalDeviceFeatures2 f2{};

        vk::PhysicalDeviceVulkan11Features v11f{};
        vk::PhysicalDeviceVulkan12Features v12f{};
        vk::PhysicalDeviceVulkan13Features v13f{};

        ci.setPEnabledExtensionNames(extensions).setQueueCreateInfos(queueCreateInfos);

        auto ci_structchain = vk::StructureChain(ci, f2, v11f, v12f, v13f);

        return globalState->physicalDevice.createDevice(ci_structchain.get());
    }

    void gatherQueues() {
        globalState->primaryQueue = globalState->device.getQueue(globalState->primaryQueueFamily, 0);
        globalState->transferQueue = globalState->device.getQueue(globalState->transferQueueFamily, 0);
    }
}// namespace kat
