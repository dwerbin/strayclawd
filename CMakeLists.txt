cmake_minimum_required(VERSION 3.20)
project(ClaudePoo CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(claudepoo WIN32
    src/main.cpp
    src/Canvas.cpp
    src/Scanner.cpp
    src/Mate.cpp
    src/Tray.cpp
    src/Settings.cpp
)

target_compile_definitions(claudepoo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(claudepoo PRIVATE /W4 /utf-8 /permissive-)
target_link_libraries(claudepoo PRIVATE user32 gdi32 dwmapi shell32 advapi32 shcore)
