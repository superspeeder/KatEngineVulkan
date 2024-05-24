cmake_minimum_required(VERSION 3.28)
project(KatEngineVulkan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(libs)
add_subdirectory(engine)
add_subdirectory(game_sample)
