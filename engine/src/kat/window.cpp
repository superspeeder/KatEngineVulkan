#include "window.hpp"
#include <spdlog/spdlog.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

namespace kat {
    void closefun(GLFWwindow *win) {
        auto window = kat::getWindow(win);
        if (window) {
            bool shouldClose = true;
            window->OnShouldCloseSignal.publish(window, &shouldClose);

            if (!shouldClose) {
                glfwSetWindowShouldClose(win, false);
                return;
            }

            window->OnCloseSignal.publish(window);
        }
    }

    void sizefun(GLFWwindow *win, int width, int height) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnWindowResizeSignal.publish(window, vk::Extent2D{(uint32_t) width, (uint32_t) height});
        }
    }

    void fbsizefun(GLFWwindow *win, int width, int height) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnResizeSignal.publish(window, vk::Extent2D{(uint32_t) width, (uint32_t) height});
        }
    }

    void cscfun(GLFWwindow *win, float xscl, float yscl) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnContentScaleChangedSignal.publish(window, glm::vec2{xscl, yscl});
        }
    }

    void wpfun(GLFWwindow *win, int x, int y) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnWindowMoveSignal.publish(window, glm::ivec2{x, y});
        }
    }

    void icfun(GLFWwindow *win, int iconified) {
        auto window = kat::getWindow(win);
        if (window) {
            if (iconified) {
                window->OnIconifiedSignal.publish(window);
            } else {
                window->OnRestoredSignal.publish(window, RestoredFrom::ICONIFIED);
            }
        }
    }

    void mxfun(GLFWwindow *win, int maximized) {
        auto window = kat::getWindow(win);
        if (window) {
            if (maximized) {
                window->OnMaximizedSignal.publish(window);
            } else {
                window->OnRestoredSignal.publish(window, RestoredFrom::MAXIMIZED);
            }
        }
    }

    void fcfun(GLFWwindow *win, int focused) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnFocusChangedSignal.publish(window, focused == GLFW_TRUE);
        }
    }

    void reffun(GLFWwindow *win) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnRedrawSignal.publish(window);
        }
    }

    void mbfun(GLFWwindow *win, int button, int action, int mods) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnMouseButtonSignal.publish(window, button, action == GLFW_PRESS, mods);
        }
    }

    void cposfun(GLFWwindow *win, double x, double y) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnMouseMovedSignal.publish(window, glm::dvec2(x, y));
        }
    }

    void centfun(GLFWwindow *win, int ent) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnCursorHoverChangedSignal.publish(window, ent == GLFW_TRUE);
        }
    }

    void scfun(GLFWwindow *win, double xo, double yo) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnScrollSignal.publish(window, glm::dvec2(xo, yo));
        }
    }

    void kfun(GLFWwindow *win, int key, int scancode, int action, int mods) {
        auto window = kat::getWindow(win);
        if (window) {
            if (action == GLFW_REPEAT) return;
            window->OnKeySignal.publish(window, key, scancode, action == GLFW_PRESS, mods);
        }
    }

    void chfun(GLFWwindow *win, unsigned int codepoint) {
        auto window = kat::getWindow(win);
        if (window) {
            window->OnCharSignal.publish(window, codepoint);
        }
    }

    void dropfun(GLFWwindow *win, int path_count, const char **paths) {
        auto window = kat::getWindow(win);
        if (window) {
            std::vector<std::string_view> ps;
            ps.reserve(path_count);
            for (int i = 0; i < path_count; i++) {
                ps.emplace_back(paths[i]);
            }

            window->OnPathsDropSignal.publish(window, ps);
        }
    }

    void causeRedraw(GLFWwindow *win) {
        HWND hwnd = glfwGetWin32Window(win);
        RedrawWindow(hwnd, nullptr, nullptr, RDW_INTERNALPAINT | RDW_UPDATENOW);
    }

    Window::Window(_In_ const WindowSettings &settings, _In_ size_t id) : m_Id(id) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, settings.style.resizable);
        glfwWindowHint(GLFW_VISIBLE, settings.style.visible);
        glfwWindowHint(GLFW_DECORATED, settings.style.decorated);
        glfwWindowHint(GLFW_FOCUSED, settings.style.focused);
        glfwWindowHint(GLFW_AUTO_ICONIFY, settings.style.autoIconify);
        glfwWindowHint(GLFW_FLOATING, settings.style.floating);
        glfwWindowHint(GLFW_MAXIMIZED, settings.style.maximized);
        glfwWindowHint(GLFW_CENTER_CURSOR, settings.style.centerCursor);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, settings.style.transparentFramebuffer);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, settings.style.focusOnShow);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, settings.style.scaleToMonitor);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, settings.style.mousePassthrough);
        glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, settings.style.keyboardMenu);
        glfwWindowHint(GLFW_WIN32_SHOWDEFAULT, settings.style.showDefault);
        glfwWindowHint(GLFW_POSITION_X, settings.position.x);
        glfwWindowHint(GLFW_POSITION_Y, settings.position.y);

        m_Window = glfwCreateWindow(settings.size.width, settings.size.height, settings.title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetWindowCloseCallback(m_Window, closefun);
        glfwSetWindowSizeCallback(m_Window, sizefun);
        glfwSetFramebufferSizeCallback(m_Window, fbsizefun);
        glfwSetWindowContentScaleCallback(m_Window, cscfun);
        glfwSetWindowPosCallback(m_Window, wpfun);
        glfwSetWindowIconifyCallback(m_Window, icfun);
        glfwSetWindowMaximizeCallback(m_Window, mxfun);
        glfwSetWindowFocusCallback(m_Window, fcfun);
        glfwSetWindowRefreshCallback(m_Window, reffun);
        glfwSetMouseButtonCallback(m_Window, mbfun);
        glfwSetCursorPosCallback(m_Window, cposfun);
        glfwSetCursorEnterCallback(m_Window, centfun);
        glfwSetScrollCallback(m_Window, scfun);
        glfwSetKeyCallback(m_Window, kfun);
        glfwSetCharCallback(m_Window, chfun);
        glfwSetDropCallback(m_Window, dropfun);
    }

    Window::~Window() {
        if (m_Window) {
            cleanup();

            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
    }

    GLFWwindow *Window::getHandle() const noexcept {
        return m_Window;
    }

    size_t Window::getId() const noexcept {
        return m_Id;
    }

    const vk::SurfaceKHR &Window::getSurface() const {
        return m_Surface;
    }

    void Window::cleanup() {
        if (m_Surface)
            globalState->vkInstance.destroySurfaceKHR(m_Surface, nullptr, globalState->dldy);

        spdlog::debug("Cleaned up window internals.");
    }

    void Window::redraw() {
        causeRedraw(m_Window);
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::setUserPointer(size_t index, void *ptr) {
        m_UserPointers[index] = ptr;
    }

    void *Window::getUserPointer(size_t index) const {
        return m_UserPointers[index];
    }

    void Window::resizeUserPointerArray(size_t size) {
        m_UserPointers.resize(size);
    }

    void Window::requestClose() {
        bool shouldClose = true;
        OnShouldCloseSignal.publish(this, &shouldClose);
        if (shouldClose) {
            close();
        }
    }

    void Window::close() {
        glfwSetWindowShouldClose(m_Window, true);
        OnCloseSignal.publish(this);
    }
}// namespace kat