#include "game.hpp"
#include "kat/core.hpp"
#include "kat/window.hpp"

#include <kat/consrv.hpp>
#include <spdlog/spdlog.h>

bool check = false;
bool recreate = false;

void aboutToCloseWinhandler() {
    if (kat::windowCount() == 1 && !check) {
        check = true;
        recreate = true;
        kat::setKeepOpen(true);
        spdlog::info("Last window closing first time. Setting keep open for recreation.");
    }
}

void closeWinhandler() {
    spdlog::info("Close handler");
    if (recreate) {
        spdlog::info("Recreating last window.");
        recreate = false;

        kat::WindowSettings windowSettings{};
        windowSettings.title = L"Sample Game 3";
        kat::createWindow(windowSettings);

        kat::setKeepOpen(false);
        spdlog::info("Keep open off.");
    }
}

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

    windowSettings.title = L"Sample Game 2";
    size_t window2 = kat::createWindow(windowSettings);

    kat::start();

    kat::getWindow(window)->OnClose.connect<&aboutToCloseWinhandler>();
    kat::getWindow(window2)->OnClose.connect<&aboutToCloseWinhandler>();

    kat::OnNoWindowsOpen.connect<&closeWinhandler>();

    MSG msg{};
    std::optional<int> ec;
    while (!ec.has_value()) {
        ec = kat::pollMessages(&msg);
    }

    return ec.value_or(EXIT_SUCCESS);
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
    return ec;
}