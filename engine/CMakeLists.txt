cmake_minimum_required(VERSION 3.28)

project(katengine_engine VERSION 0.1.0 LANGUAGES CXX)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined glslc)

configure_file(config.hpp.in incl/kat/config.hpp @ONLY)

add_library(katengine src/kat/core.cpp src/kat/core.hpp
        src/kat/window.cpp
        src/kat/window.hpp)

target_include_directories(katengine PUBLIC src/ ${CMAKE_CURRENT_BINARY_DIR}/incl)
target_link_libraries(katengine PUBLIC spdlog::spdlog Vulkan::Vulkan Vulkan::shaderc_combined EnTT::EnTT glm::glm glfw)
target_compile_definitions(katengine PUBLIC -DUNICODE -DSPDLOG_WCHAR_TO_UTF8_SUPPORT -DVK_ENABLE_BETA_EXTENSIONS)


add_library(kat::engine ALIAS katengine)