@echo off
setlocal enabledelayedexpansion
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo vswhere.exe not found: install Visual Studio 2026 with the C++ workload.
    exit /b 1
)
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
    echo MSVC x64 tools not found.
    exit /b 1
)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

if not defined WindowsSdkDir (
    set "KITS=%ProgramFiles(x86)%\Windows Kits\10"
    set "SDKVER="
    for /f "delims=" %%v in ('dir /b /ad /on "!KITS!\Include\10.*" 2^>nul') do (
        if exist "!KITS!\Lib\%%v\um\x64\user32.lib" set "SDKVER=%%v"
    )
    if not defined SDKVER (
        echo Windows 10/11 SDK not found.
        exit /b 1
    )
    echo Using Windows SDK !SDKVER! found on disk.
    set "INCLUDE=!INCLUDE!;!KITS!\Include\!SDKVER!\ucrt;!KITS!\Include\!SDKVER!\um;!KITS!\Include\!SDKVER!\shared;!KITS!\Include\!SDKVER!\winrt"
    set "LIB=!LIB!;!KITS!\Lib\!SDKVER!\ucrt\x64;!KITS!\Lib\!SDKVER!\um\x64"
    set "PATH=!PATH!;!KITS!\bin\!SDKVER!\x64"
)

cmake -S "%~dp0." -B "%~dp0build" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build "%~dp0build" || exit /b 1
echo.
echo Built: %~dp0build\clawdmate.exe
