cmake_minimum_required(VERSION 3.20)
project(StrayClawd CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(clawdmate WIN32
    src/main.cpp
    src/Canvas.cpp
    src/Scanner.cpp
    src/Mate.cpp
    src/Tray.cpp
    src/Settings.cpp
    src/ConfigDialog.cpp
)

target_compile_definitions(clawdmate PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(clawdmate PRIVATE /W4 /utf-8 /permissive-)
target_link_libraries(clawdmate PRIVATE user32 gdi32 dwmapi shell32 advapi32 shcore gdiplus ole32 comctl32)

add_executable(render_poses EXCLUDE_FROM_ALL tools/render_poses.cpp src/Canvas.cpp)
target_compile_definitions(render_poses PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(render_poses PRIVATE /W4 /utf-8 /permissive-)
target_link_libraries(render_poses PRIVATE user32 gdi32 gdiplus ole32)
