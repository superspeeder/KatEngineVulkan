cmake_minimum_required(VERSION 3.28)

add_subdirectory(katconsrvlib)

set(SPDLOG_WCHAR_SUPPORT ON)
add_subdirectory(spdlog)

set(ENTT_INCLUDE_NATVIS ON)
add_subdirectory(entt)

add_subdirectory(glm)