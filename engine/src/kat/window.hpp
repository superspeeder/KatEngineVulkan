#pragma once

#include "kat/core.hpp"

namespace kat {

    struct WindowStyle {
        // normal WS_
        bool border = true;
        bool caption = true;
        bool clipChildren = false;
        bool clipSiblings = false;
        bool disabled = false;
        bool dialogFrame = false;
        bool maximized = false;
        bool iconic = false;
        bool maximizeButton = true;
        bool minimizeButton = true;
        bool popup = false;
        bool thickFrame = true;
        bool tabStop = false;
        bool visible = false;
        bool sysMenu = false;

        // extended WS_EX
        bool acceptFiles = false;
        bool appWindow = false;
        bool clientEdge = true;
        bool composited = false;
        bool contextHelp = false;
        bool dialogModalFrame = false;
        bool noActivate = false;
        bool noInheritLayout = false;
        bool staticEdge = false;
        bool toolWindow = false;
        bool topmost = false;
        bool transparent = false;
        bool windowEdge = true;

        [[nodiscard]] DWORD winStyle() const noexcept;
        [[nodiscard]] DWORD winStyleEx() const noexcept;

    };

    struct WindowSettings {
        std::wstring title = L"Window";
        vk::Extent2D size{800, 600};
        glm::ivec2 position{CW_USEDEFAULT, CW_USEDEFAULT};

        WindowStyle style{};
    };

    enum class ResizeMode {
        MAX_HIDE = SIZE_MAXHIDE,
        MAXIMIZED = SIZE_MAXIMIZED,
        MAX_SHOW = SIZE_MAXSHOW,
        MINIMIZED = SIZE_MINIMIZED,
        RESTORED = SIZE_RESTORED,
    };

    enum class ResizeEdge {
        BOTTOM = WMSZ_BOTTOM,
        BOTTOM_LEFT = WMSZ_BOTTOMLEFT,
        BOTTOM_RIGHT = WMSZ_BOTTOMRIGHT,
        LEFT = WMSZ_LEFT,
        RIGHT = WMSZ_RIGHT,
        TOP = WMSZ_TOP,
        TOP_LEFT = WMSZ_TOPLEFT,
        TOP_RIGHT = WMSZ_TOPRIGHT,
    };

    enum class StyleChangeMode {
        STYLE = GWL_STYLE,
        EXSTYLE = GWL_EXSTYLE,
        STYLE_AND_EXSTYLE = GWL_STYLE | GWL_EXSTYLE,
    };

    enum class ActivateMode {
        ACTIVE = WA_ACTIVE,
        CLICK_ACTIVE = WA_CLICKACTIVE,
        INACTIVE = WA_INACTIVE,
    };

    struct HotkeyMods {
        bool alt, control, shift, win;
    };

    using FNONAPPCOMMAND = void(short cmd, WORD uDevice, DWORD dwKeys);

    struct KeyEventInfo {
        WORD vkCode;
        WORD keyFlags;
        WORD scanCode;
        bool isExtendedKey;
        bool wasKeyDown;
        WORD repeatCount;
        bool isKeyReleased;
    };

    enum class SystemCommand {
        CLOSE = SC_CLOSE,
        CONTEXTHELP = SC_CONTEXTHELP,
        DEFAULT = SC_DEFAULT,
        HOTKEY = SC_HOTKEY,
        HSCROLL = SC_HSCROLL,
        ISSECURE = SCF_ISSECURE,
        KEYMENU = SC_KEYMENU,
        MAXIMIZE = SC_MAXIMIZE,
        MINIMIZE = SC_MINIMIZE,
        MONITORPOWER = SC_MONITORPOWER,
        MOUSEMENU = SC_MOUSEMENU,
        MOVE = SC_MOVE,
        NEXTWINDOW = SC_NEXTWINDOW,
        PREVWINDOW = SC_PREVWINDOW,
        RESTORE = SC_RESTORE,
        SCREENSAVE = SC_SCREENSAVE,
        SIZE = SC_SIZE,
        TASKLIST = SC_TASKLIST,
        VSCROLL = SC_VSCROLL,
    };

    struct ModifierKeyInfo {
        bool control, leftButton, middleButton, rightButton, shift, xButton1, xButton2, alt;
    };

    KeyEventInfo procKeyEventInfo(WPARAM wParam, LPARAM lParam);

    class Window {
      public:
        Window(_In_ const WindowSettings &settings, _In_ size_t id);
        ~Window();

        [[nodiscard]] HWND getHandle() const noexcept;

        void setStyle(_In_ const WindowStyle& style);

        static LRESULT CALLBACK globalProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        [[nodiscard]] size_t getId() const noexcept;

        [[nodiscard]] const vk::SurfaceKHR &getSurface() const;

        void cleanup();

        KAT_SIGNAL(OnClose, void());
        KAT_SIGNAL(OnShouldClose, void(bool*));

        KAT_SIGNAL(OnLowMemory, void());
        KAT_SIGNAL(OnEnabledChanged, void(bool enabled));
        KAT_SIGNAL(OnEnable, void());
        KAT_SIGNAL(OnDisable, void());
        KAT_SIGNAL(OnMove, void(const glm::ivec2&));
        KAT_SIGNAL(OnMoving, void(RECT*));
        KAT_SIGNAL(OnShowWindow, void(bool));
        KAT_SIGNAL(OnResize, void(ResizeMode, const vk::Extent2D&));
        KAT_SIGNAL(OnResizing, void(ResizeEdge, RECT*));
        KAT_SIGNAL(OnStyleChanged, void(StyleChangeMode, const STYLESTRUCT*));
        KAT_SIGNAL(OnStyleChanging, void(StyleChangeMode, STYLESTRUCT*));
        KAT_SIGNAL(OnThemeChanged, void());
        KAT_SIGNAL(OnActivateApp, void(bool));
        KAT_SIGNAL(OnActivateWindow, void(ActivateMode));
        KAT_SIGNAL(OnAppCommand, FNONAPPCOMMAND);
        KAT_SIGNAL(OnChar, void(char));
        KAT_SIGNAL(OnDeadChar, void(char));
        KAT_SIGNAL(OnHotkey, void(WPARAM, WORD, HotkeyMods));
        KAT_SIGNAL(OnKeyPressed, void(KeyEventInfo));
        KAT_SIGNAL(OnKeyReleased, void(KeyEventInfo));
        KAT_SIGNAL(OnKillFocus, void());
        KAT_SIGNAL(OnSetFocus, void());
        KAT_SIGNAL(OnSysDeadChar, void(char));
        KAT_SIGNAL(OnSysKeyPressed, void(KeyEventInfo));
        KAT_SIGNAL(OnSysKeyReleased, void(KeyEventInfo));
        KAT_SIGNAL(OnUnicodeChar, void(wchar_t));
        KAT_SIGNAL(OnSysChar, void(char));
        KAT_SIGNAL(OnSysCommand, void(SystemCommand, glm::ivec2));
        KAT_SIGNAL(OnDisplayChange, void(UINT, vk::Extent2D));
        KAT_SIGNAL(OnNonClientPaint, void(HRGN));
        KAT_SIGNAL(OnPaint, void());
        KAT_SIGNAL(OnDropFiles, void(HDROP));
        KAT_SIGNAL(OnHelp, void(HELPINFO*));
        KAT_SIGNAL(OnCommand, void(WPARAM, LPARAM)); // see WM_COMMAND
        KAT_SIGNAL(OnContextMenu, void(glm::ivec2));
        KAT_SIGNAL(OnTimer, void(WPARAM, LPARAM));
        KAT_SIGNAL(OnHScrollRaw, void(WPARAM, LPARAM));
        KAT_SIGNAL(OnVScrollRaw, void(WPARAM, LPARAM));

      private:
        HWND m_Handle;
        size_t m_Id;
        vk::SurfaceKHR m_Surface;

        void onActivateApp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onClose(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onCompacting(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onEnable(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMove(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMoving(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onShowWindow(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSize(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSizing(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onStyleChanged(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onStyleChanging(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onThemeChanged(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onActivate(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onAppCommand(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onChar(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onDeadChar(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onHotkey(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onKeyUp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onKillFocus(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSetFocus(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSysDeadChar(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSysKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSysKeyUp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onUniChar(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSysChar(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onSysCommand(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onDisplayChange(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onNcPaint(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onPaint(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onDropFiles(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onHelp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onCommand(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onContextMenu(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onTimer(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onHScroll(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onVScroll(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMouseMove(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onLButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onLButtonUp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onRButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onRButtonUp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMButtonUp(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMouseWheel(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMouseHWheel(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onCaptureChanged(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMouseHover(_In_ WPARAM wParam, _In_ LPARAM lParam);
        void onMouseLeave(_In_ WPARAM wParam, _In_ LPARAM lParam);
    };

}// namespace kat
