#include "Scanner.h"

#include <dwmapi.h>
#include <cwchar>

namespace {

bool IsCloaked(HWND hwnd)
{
    DWORD cloaked = 0;
    return SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked != 0;
}

}

void Scanner::Refresh()
{
    wins_.clear();
    EnumWindows(&Scanner::EnumProc, reinterpret_cast<LPARAM>(this));
}

BOOL CALLBACK Scanner::EnumProc(HWND hwnd, LPARAM param)
{
    auto* self = reinterpret_cast<Scanner*>(param);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId()) {
        return TRUE;
    }
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd) || IsCloaked(hwnd)) {
        return TRUE;
    }
    const LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (ex & WS_EX_TRANSPARENT) {
        return TRUE;
    }
    wchar_t cls[64] = {};
    GetClassNameW(hwnd, cls, 64);
    if (std::wcscmp(cls, L"Progman") == 0 || std::wcscmp(cls, L"WorkerW") == 0) {
        return TRUE;
    }
    const bool taskbar = std::wcscmp(cls, L"Shell_TrayWnd") == 0 || std::wcscmp(cls, L"Shell_SecondaryTrayWnd") == 0;
    if (!taskbar && ((ex & WS_EX_TOOLWINDOW) || GetWindowTextLengthW(hwnd) == 0)) {
        return TRUE;
    }
    RECT r{};
    if (!GetFrame(hwnd, &r) || r.right <= r.left || r.bottom <= r.top) {
        return TRUE;
    }
    const bool surface = !taskbar && !(ex & WS_EX_TOPMOST) && r.right - r.left >= 300 && r.bottom - r.top >= 200;
    self->wins_.push_back({hwnd, r, surface});
    return TRUE;
}

bool Scanner::IsCovered(POINT pt, size_t below) const
{
    for (size_t i = 0; i < below && i < wins_.size(); ++i) {
        if (PtInRect(&wins_[i].rect, pt)) {
            return true;
        }
    }
    return false;
}

bool Scanner::GetFrame(HWND hwnd, RECT* rect)
{
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(*rect)))) {
        return true;
    }
    return GetWindowRect(hwnd, rect) != FALSE;
}

bool Scanner::IsAlive(HWND hwnd)
{
    return hwnd && IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd) && !IsCloaked(hwnd);
}

bool Scanner::OnScreen(POINT pt)
{
    return MonitorFromPoint(pt, MONITOR_DEFAULTTONULL) != nullptr;
}
