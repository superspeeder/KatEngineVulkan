#include "game.hpp"
#include "kat/core.hpp"
#include "kat/window.hpp"

#include <conio.h>
#include <kat/consrv.hpp>
#include <spdlog/spdlog.h>

#ifdef WINDOWS_APP
#define BUILDING_WINDOWS_APP true
#else
#define BUILDING_WINDOWS_APP false
#endif

void onKey(kat::Window* window, int key, bool pressed, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        window->requestClose();
    }
}

int run() {
    kat::EngineInitInfo initInfo{
            .enableDebug = true
    };
    kat::init(initInfo);

    kat::WindowSettings windowSettings{};
    windowSettings.title = "Sample Game";

    size_t window = kat::createWindow(windowSettings);

    kat::getWindow(window)->OnKey.connect<&onKey>();

    kat::start();

    std::optional<int> ec;
    while (!ec.has_value()) {
        ec = kat::pollMessages();
        kat::updateWindows();
    }

    return ec.value_or(EXIT_SUCCESS);
}

bool isConsoleOwner() {
    HWND hwnd = GetConsoleWindow();
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    DWORD cur_pid = GetCurrentProcessId();

    return pid == cur_pid;
}

void consoleShowTerminate(int ec) {
    // backup method to make sure that we can manage a system which doesn't have a copy of katconsrv.exe available.
    printf("\nExited with code %d\n\nPress any key to exit....\n", ec);
    while (!_kbhit()) std::this_thread::yield();
}

#ifdef WINDOWS_APP
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ const LPWSTR lpCmdLine, // NOLINT(*-non-const-parameter, *-misplaced-const)
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(nCmdShow);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
#else
int main() {
#endif

#ifdef WINDOWS_APP
    kat::consrv::RedirectConsoleIO(); // unnecessary if we are in console mode
#endif

    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Hello!");

    int ec = EXIT_FAILURE;
    try {
        ec = run();
    } catch (const std::exception& e) {
        spdlog::critical(e.what());
    }

    kat::terminate();

#ifdef WINDOWS_APP
    if (isConsoleOwner()) {
        consoleShowTerminate(ec);
    }
#endif
    return ec;
}