#include "window.hpp"
#include <spdlog/spdlog.h>
#include <windowsx.h>

namespace kat {
    DWORD WindowStyle::winStyle() const noexcept {
        DWORD ws = 0;
        if (border) ws |= WS_BORDER;
        if (caption) ws |= WS_CAPTION;
        if (clipChildren) ws |= WS_CLIPCHILDREN;
        if (clipSiblings) ws |= WS_CLIPSIBLINGS;
        if (disabled) ws |= WS_DISABLED;
        if (dialogFrame) ws |= WS_DLGFRAME;
        if (maximized) ws |= WS_MAXIMIZE;
        if (iconic) ws |= WS_ICONIC;
        if (maximizeButton) ws |= WS_MAXIMIZEBOX | WS_SYSMENU;
        if (minimizeButton) ws |= WS_MINIMIZEBOX | WS_SYSMENU;
        if (popup) ws |= WS_POPUP;
        if (thickFrame) ws |= WS_THICKFRAME;
        if (tabStop) ws |= WS_TABSTOP;
        if (visible) ws |= WS_BORDER;
        if (sysMenu) ws |= WS_SYSMENU;

        return ws;
    }

    DWORD WindowStyle::winStyleEx() const noexcept {
        DWORD ws_ex = 0;

        if (acceptFiles) ws_ex |= WS_EX_ACCEPTFILES;
        if (appWindow) ws_ex |= WS_EX_APPWINDOW;
        if (clientEdge) ws_ex |= WS_EX_CLIENTEDGE;
        if (composited) ws_ex |= WS_EX_COMPOSITED;
        if (contextHelp) ws_ex |= WS_EX_CONTEXTHELP;
        if (dialogModalFrame) ws_ex |= WS_EX_DLGMODALFRAME;
        if (noActivate) ws_ex |= WS_EX_NOACTIVATE;
        if (noInheritLayout) ws_ex |= WS_EX_NOINHERITLAYOUT;
        if (staticEdge) ws_ex |= WS_EX_STATICEDGE;
        if (toolWindow) ws_ex |= WS_EX_TOOLWINDOW;
        if (topmost) ws_ex |= WS_EX_TOPMOST;
        if (transparent) ws_ex |= WS_EX_TRANSPARENT;
        if (windowEdge) ws_ex |= WS_EX_WINDOWEDGE;

        return ws_ex;
    }

    Window::Window(_In_ const WindowSettings &settings, _In_ size_t id) : m_Id(id) {
        DWORD ws = settings.style.winStyle();
        DWORD wsex = settings.style.winStyleEx();
        m_Handle = CreateWindowExW(wsex, WCNAME, settings.title.c_str(), ws, settings.position.x, settings.position.y, settings.size.width, settings.size.height, nullptr, nullptr, globalState->hInstance, this);

        vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{{}, globalState->hInstance, m_Handle};
        m_Surface = globalState->vkInstance.createWin32SurfaceKHR(surfaceCreateInfo, nullptr, globalState->dldy);

        ShowWindow(m_Handle, globalState->nCmdShow);
        UpdateWindow(m_Handle);
    }

    Window::~Window() {
        if (IsWindow(m_Handle)) DestroyWindow(m_Handle);
    }

    HWND Window::getHandle() const noexcept {
        return m_Handle;
    }

    void Window::setStyle(_In_ const WindowStyle &style) {
        SetWindowLong(m_Handle, GWL_STYLE, style.winStyle());
        SetWindowLong(m_Handle, GWL_EXSTYLE, style.winStyleEx());
    }

    void globalOnCreate(_In_ HWND hwnd, _In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto *cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        if (cs->lpCreateParams) {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        }
    }

    LRESULT Window::globalProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
#define PROCHELPER(name) window->name(wParam, lParam)
        switch (msg) {
            case WM_ACTIVATEAPP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onActivateApp);
                }
            } break;
            case WM_CLOSE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onClose);
                }
            } break;
            case WM_COMPACTING: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onCompacting);
                }
            } break;
            case WM_CREATE:
                globalOnCreate(hwnd, wParam, lParam);
                break;
            case WM_DESTROY:
                destroyWindow(hwnd);
                break;
            case WM_ENABLE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onEnable);
                }
            } break;
            case WM_MOVE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMove);
                }
            } break;
            case WM_MOVING: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMoving);
                }
                return TRUE;
            }
            case WM_SHOWWINDOW: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onShowWindow);
                }
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }
            case WM_SIZE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSize);
                }
            } break;
            case WM_SIZING: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSizing);
                }
                return TRUE;
            }
            case WM_STYLECHANGED: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onStyleChanged);
                }
            } break;
            case WM_STYLECHANGING: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onStyleChanging);
                }
            } break;
            case WM_THEMECHANGED: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onThemeChanged);
                }
            } break;

            case WM_ACTIVATE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onActivate);
                }
            } break;
            case WM_APPCOMMAND: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onAppCommand);
                }
            } break;
            case WM_CHAR: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onChar);
                }
            } break;
            case WM_DEADCHAR: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onDeadChar);
                }
            } break;
            case WM_HOTKEY: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onHotkey);
                }
            } break;
            case WM_KEYDOWN: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onKeyDown);
                }
            } break;
            case WM_KEYUP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onKeyUp);
                }
            } break;
            case WM_KILLFOCUS: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onKillFocus);
                }
            } break;
            case WM_SETFOCUS: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSetFocus);
                }
            } break;
            case WM_SYSDEADCHAR: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSysDeadChar);
                }
            } break;
            case WM_SYSKEYDOWN: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSysKeyDown);
                }
            } break;
            case WM_SYSKEYUP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSysKeyUp);
                }
            } break;
            case WM_UNICHAR: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onUniChar);
                }
            } break;

            case WM_SYSCHAR: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSysChar);
                }
            } break;

            case WM_SYSCOMMAND: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onSysCommand);
                }
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }

            case WM_DISPLAYCHANGE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onDisplayChange);
                }
            } break;
            case WM_NCPAINT: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onNcPaint);
                }
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }
            case WM_PAINT: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onPaint);
                }
            } break;

            case WM_DROPFILES: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onDropFiles);
                }
            } break;
            case WM_HELP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onHelp);
                }
                return TRUE;
            }

            case WM_COMMAND: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onCommand);
                }
            } break;
            case WM_CONTEXTMENU: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onContextMenu);
                }
            } break;

            case WM_TIMER: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onTimer);
                }
            } break;

            case WM_HSCROLL: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onHScroll);
                }
            } break;
            case WM_VSCROLL: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onVScroll);
                }
            } break;

            case WM_MOUSEMOVE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMouseMove);
                }
            } break;
            case WM_LBUTTONDOWN: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onLButtonDown);
                }
            } break;
            case WM_LBUTTONUP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onLButtonUp);
                }
            } break;
            case WM_RBUTTONDOWN: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onRButtonDown);
                }
            } break;
            case WM_RBUTTONUP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onRButtonUp);
                }
            } break;
            case WM_MBUTTONDOWN: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMButtonDown);
                }
            } break;
            case WM_MBUTTONUP: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMButtonUp);
                }
            } break;
            case WM_MOUSEWHEEL: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMouseWheel);
                }
            } break;
            case WM_MOUSEHWHEEL: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMouseHWheel);
                }
            } break;

            case WM_CAPTURECHANGED: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onCaptureChanged);
                }
            } break;

            case WM_MOUSEHOVER: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMouseHover);
                }
            } break;
            case WM_MOUSELEAVE: {
                Window *window = getWindow(hwnd);
                if (window) {
                    PROCHELPER(onMouseLeave);
                }
            } break;

            default:
                return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
        return 0;
#undef PROCHELPER
    }

    void Window::onActivateApp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        bool activated = wParam == TRUE;
        OnActivateAppSignal.publish(activated);
    }

    void Window::onClose(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        bool shouldClose = true;
        OnShouldCloseSignal.publish(&shouldClose);

        if (shouldClose) {
            OnCloseSignal.publish();
            DestroyWindow(m_Handle);
            // NO MORE CODE AFTER HERE (this may no longer exist).
        }
    }

    void Window::onCompacting(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnLowMemorySignal.publish();
    }

    void Window::onEnable(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnEnabledChangedSignal.publish(wParam == TRUE);
        if (wParam == TRUE) {
            OnEnableSignal.publish();
        } else {
            OnDisableSignal.publish();
        };
    }

    void Window::onMove(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        int x = (int) (short) LOWORD(lParam);
        int y = (int) (short) HIWORD(lParam);
        OnMoveSignal.publish(glm::ivec2(x, y));
    }

    void Window::onMoving(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto* pRect = reinterpret_cast<RECT*>(lParam);
        OnMovingSignal.publish(pRect);
    }

    void Window::onShowWindow(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnShowWindowSignal.publish(wParam == TRUE);
    }

    void Window::onSize(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto mode = static_cast<ResizeMode>(wParam);
        vk::Extent2D extent = { LOWORD(lParam), HIWORD(lParam) };
        OnResizeSignal.publish(mode, extent);
    }

    void Window::onSizing(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto edge = static_cast<ResizeEdge>(wParam);
        auto* pRect = reinterpret_cast<RECT*>(lParam);
        OnResizingSignal.publish(edge, pRect);
    }

    void Window::onStyleChanged(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto mode = static_cast<StyleChangeMode>(wParam);
        const auto* ss = reinterpret_cast<const STYLESTRUCT*>(lParam);
        OnStyleChangedSignal.publish(mode, ss);
    }

    void Window::onStyleChanging(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto mode = static_cast<StyleChangeMode>(wParam);
        auto* ss = reinterpret_cast<STYLESTRUCT*>(lParam);
        OnStyleChangingSignal.publish(mode, ss);
    }

    void Window::onThemeChanged(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnThemeChangedSignal.publish();
    }

    void Window::onActivate(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto activeMode = static_cast<ActivateMode>(wParam);
        OnActivateWindowSignal.publish(activeMode);
    }

    void Window::onAppCommand(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        short cmd = GET_APPCOMMAND_LPARAM(lParam);
        WORD uDevice = GET_DEVICE_LPARAM(lParam);
        DWORD dwKeys = GET_KEYSTATE_LPARAM(lParam);
        OnAppCommandSignal.publish(cmd, uDevice, dwKeys);
    }

    void Window::onChar(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnCharSignal.publish(wParam);
    }

    void Window::onDeadChar(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnDeadCharSignal.publish(wParam);
    }

    void Window::onHotkey(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        WORD mods = LOWORD(lParam);
        WORD vkCode = HIWORD(lParam);

        HotkeyMods hotkeyMods{};
        hotkeyMods.alt = mods & MOD_ALT;
        hotkeyMods.control = mods & MOD_CONTROL;
        hotkeyMods.shift = mods & MOD_SHIFT;
        hotkeyMods.win = mods & MOD_WIN;

        OnHotkeySignal.publish(wParam, vkCode, hotkeyMods);
    }

    void Window::onKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        KeyEventInfo kei = procKeyEventInfo(wParam, lParam);
        OnKeyPressedSignal.publish(kei);
    }

    void Window::onKeyUp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        KeyEventInfo kei = procKeyEventInfo(wParam, lParam);
        OnKeyReleasedSignal.publish(kei);
    }

    void Window::onKillFocus(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnKillFocusSignal.publish();
    }

    void Window::onSetFocus(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnSetFocusSignal.publish();
    }

    void Window::onSysDeadChar(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnSysDeadCharSignal.publish(wParam);
    }

    void Window::onSysKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        KeyEventInfo kei = procKeyEventInfo(wParam, lParam);
        OnSysKeyPressedSignal.publish(kei);
    }

    void Window::onSysKeyUp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        KeyEventInfo kei = procKeyEventInfo(wParam, lParam);
        OnSysKeyReleasedSignal.publish(kei);
    }

    void Window::onUniChar(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnUnicodeCharSignal.publish(wParam);
    }

    void Window::onSysChar(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnSysCharSignal.publish(wParam);
    }

    void Window::onSysCommand(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        auto sc = static_cast<SystemCommand>(wParam);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        OnSysCommandSignal.publish(sc, glm::ivec2(x, y));
    }

    void Window::onDisplayChange(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        vk::Extent2D size = { LOWORD(lParam), HIWORD(lParam) };
        UINT bpp = wParam;
        OnDisplayChangeSignal.publish(bpp, size);
    }

    void Window::onNcPaint(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        HRGN hRgn = (HRGN)wParam;
        OnNonClientPaintSignal.publish(hRgn);
    }

    void Window::onPaint(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnPaintSignal.publish();
    }

    void Window::onDropFiles(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnDropFilesSignal.publish((HDROP)wParam);
    }

    void Window::onHelp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnHelpSignal.publish((HELPINFO*)lParam);
    }

    void Window::onCommand(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnCommandSignal.publish(wParam, lParam);
    }

    void Window::onContextMenu(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        OnContextMenuSignal.publish(glm::ivec2(x, y));
    }

    void Window::onTimer(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnTimerSignal.publish(wParam, lParam);
    }

    void Window::onHScroll(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnHScrollRawSignal.publish(wParam, lParam);
    }

    void Window::onVScroll(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        OnVScrollRawSignal.publish(wParam, lParam);
    }

    void Window::onMouseMove(_In_ WPARAM wParam, _In_ LPARAM lParam) {
        int xPos =
    }

    void Window::onLButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onLButtonUp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onRButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onRButtonUp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onMButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onMButtonUp(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onMouseWheel(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onMouseHWheel(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onCaptureChanged(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onMouseHover(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    void Window::onMouseLeave(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    }

    size_t Window::getId() const noexcept {
        return m_Id;
    }

    const vk::SurfaceKHR &Window::getSurface() const {
        return m_Surface;
    }

    void Window::cleanup() {
        globalState->vkInstance.destroySurfaceKHR(m_Surface, nullptr, globalState->dldy);
        spdlog::debug("Cleaned up window internals.");
    }

    KeyEventInfo procKeyEventInfo(WPARAM wParam, LPARAM lParam) {
        KeyEventInfo kei{};
        kei.vkCode = LOWORD(wParam);
        kei.keyFlags = HIWORD(lParam);
        kei.scanCode = LOBYTE(kei.keyFlags);
        kei.isExtendedKey = (kei.keyFlags & KF_EXTENDED) == KF_EXTENDED;
        if (kei.isExtendedKey) {
            kei.scanCode = MAKEWORD(kei.scanCode, 0xE0);
        }
        kei.wasKeyDown = (kei.keyFlags & KF_REPEAT) == KF_REPEAT;
        kei.repeatCount = LOWORD(lParam);
        kei.isKeyReleased = (kei.keyFlags & KF_UP) == KF_UP;

        switch (kei.vkCode)
        {
            case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
            case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
            case VK_MENU:    // converts to VK_LMENU or VK_RMENU
                kei.vkCode = LOWORD(MapVirtualKeyW(kei.scanCode, MAPVK_VSC_TO_VK_EX));
                break;
        }

        return kei;
    }
}// namespace kat