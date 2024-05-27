#pragma once

#include "kat/core.hpp"

#include <string_view>

namespace kat {
    struct WindowStyle {
        bool resizable = false;
        bool visible = true;
        bool decorated = true;
        bool focused = true;
        bool autoIconify = true;
        bool floating = false;
        bool maximized = false;
        bool centerCursor = true;
        bool transparentFramebuffer = false;
        bool focusOnShow = true;
        bool scaleToMonitor = false;
        bool mousePassthrough = false;
        bool keyboardMenu = false;
        bool showDefault = false;
    };

    struct WindowSettings {
        std::string title = "Window";
        vk::Extent2D size{800, 600};
        glm::ivec2 position{GLFW_ANY_POSITION, GLFW_ANY_POSITION};

        WindowStyle style{};
    };

    enum class RestoredFrom {
        ICONIFIED, MAXIMIZED,
    };

    class Window {
      public:
        Window(_In_ const WindowSettings &settings, _In_ size_t id);
        ~Window();

        [[nodiscard]] GLFWwindow* getHandle() const noexcept;

        [[nodiscard]] size_t getId() const noexcept;

        [[nodiscard]] const vk::SurfaceKHR &getSurface() const;

        void redraw();

        [[nodiscard]] bool shouldClose() const;

        void cleanup();

        KAT_SIGNAL(OnClose, void(Window*));
        KAT_SIGNAL(OnShouldClose, void(Window*, bool*));
        KAT_SIGNAL(OnWindowResize, void(Window*, const vk::Extent2D&));
        KAT_SIGNAL(OnResize, void(Window*, const vk::Extent2D&));
        KAT_SIGNAL(OnContentScaleChanged, void(Window*, const glm::vec2&));
        KAT_SIGNAL(OnWindowMove, void(Window*, const glm::ivec2&));
        KAT_SIGNAL(OnIconified, void(Window*));
        KAT_SIGNAL(OnMaximized, void(Window*));
        KAT_SIGNAL(OnRestored, void(Window*, RestoredFrom));
        KAT_SIGNAL(OnFocusChanged, void(Window*, bool));
        KAT_SIGNAL(OnRedraw, void(Window*));

        KAT_SIGNAL(OnMouseButton, void(Window*, int, bool, int));
        KAT_SIGNAL(OnMouseMoved, void(Window*, const glm::dvec2&));
        KAT_SIGNAL(OnCursorHoverChanged, void(Window*, bool));
        KAT_SIGNAL(OnScroll, void(Window*, const glm::dvec2&));
        KAT_SIGNAL(OnKey, void(Window*, int, int, bool, int));
        KAT_SIGNAL(OnChar, void(Window*, unsigned int));
        KAT_SIGNAL(OnPathsDrop, void(Window*, const std::vector<std::string_view>&));


        // Allow multiple user pointers so we don't have to do terrible things with our data.
        void setUserPointer(size_t index, void*);
        [[nodiscard]] void* getUserPointer(size_t index) const;

        void resizeUserPointerArray(size_t size);

        void requestClose();

        void close();

        [[nodiscard]] vk::ResultValue<uint32_t> acquireFrame(vk::Semaphore imageAvailableSemaphore);

        [[nodiscard]] vk::Image getImage(uint32_t index) const;

      private:
        GLFWwindow* m_Window;
        size_t m_Id;
        vk::SurfaceKHR m_Surface;

        std::vector<void*> m_UserPointers;

        vk::SwapchainKHR m_Swapchain;
        std::vector<vk::Image> m_Images;

        bool m_CleanedUp = false;

    };

}// namespace kat
