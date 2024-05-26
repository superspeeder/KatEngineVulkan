#include "game.hpp"
#include "kat/core.hpp"
#include "kat/window.hpp"

#include <conio.h>
#include <kat/consrv.hpp>
#include <spdlog/spdlog.h>

int run(_In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPWSTR lpCmdLine,
        _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    kat::EngineInitInfo initInfo{
            .hInstance = hInstance,
            .nCmdShow = nCmdShow,
            .enableDebug = true
    };
    kat::init(initInfo);

    kat::WindowSettings windowSettings{};
    windowSettings.title = L"Sample Game";

    size_t window = kat::createWindow(windowSettings);

    kat::start();

    MSG msg{};
    std::optional<int> ec;
    while (!ec.has_value()) {
        ec = kat::pollMessages(&msg);
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    kat::consrv::RedirectConsoleIO();
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Hello!");

    int ec = EXIT_FAILURE;
    try {
        ec = run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    } catch (const std::exception& e) {
        spdlog::critical(e.what());
    }

    kat::terminate();

    if (isConsoleOwner()) {
        // backup method to make sure that we can manage a system which doesn't have a copy of katconsrv.exe available.
        printf("\nExited with code %d\n\nPress any key to exit....\n", ec);
        while (!_kbhit()) std::this_thread::yield();
    }
    return ec;
}