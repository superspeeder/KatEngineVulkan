cmake_minimum_required(VERSION 3.28)

add_library(katconsrvlib STATIC IMPORTED GLOBAL)
set_property(TARGET katconsrvlib PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/katconsrvlib.lib")
target_include_directories(katconsrvlib INTERFACE include/)
target_link_libraries(katconsrvlib INTERFACE Shlwapi.lib)

add_library(kat::consrvlib ALIAS katconsrvlib)