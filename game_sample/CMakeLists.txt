cmake_minimum_required(VERSION 3.28)

project(katengine_game VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_WINDOWS_APP "Build a windows app instead of a console application." ON)

if (BUILD_WINDOWS_APP)
    set(GAME_SAMPLE_EXE_OPTIONS WIN32)
    set(GAME_SAMPLE_CDEFS WINDOWS_APP)
else()
    set(GAME_SAMPLE_EXE_OPTIONS)
    set(GAME_SAMPLE_CDEFS)
endif()

add_executable(game_sample ${GAME_SAMPLE_EXE_OPTIONS} src/game/game.cpp src/game/game.hpp)
target_include_directories(game_sample PRIVATE src/)
target_link_libraries(game_sample PRIVATE kat::engine kat::consrvlib spdlog::spdlog)
target_compile_definitions(game_sample PRIVATE -DUNICODE -DSPDLOG_WCHAR_TO_UTF8_SUPPORT ${GAME_SAMPLE_CDEFS})

