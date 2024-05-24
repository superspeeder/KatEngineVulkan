cmake_minimum_required(VERSION 3.28)

project(katengine_game VERSION 0.1.0 LANGUAGES CXX)

add_executable(game_sample WIN32 src/game/game.cpp src/game/game.hpp)
target_include_directories(game_sample PRIVATE src/)
target_link_libraries(game_sample PRIVATE kat::engine kat::consrvlib spdlog::spdlog)
target_compile_definitions(game_sample PRIVATE -DUNICODE -DSPDLOG_WCHAR_TO_UTF8_SUPPORT)

