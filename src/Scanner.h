#pragma once

#include <windows.h>
#include <vector>

struct WinInfo {
    HWND hwnd;
    RECT rect;
    bool surface;
};

/** Snapshot of the visible top-level windows of other processes, in z-order from top to bottom. */
class Scanner {
public:
    void Refresh();
    const std::vector<WinInfo>& Windows() const { return wins_; }
    bool IsCovered(POINT pt, size_t below) const;
    static bool GetFrame(HWND hwnd, RECT* rect);
    static bool IsAlive(HWND hwnd);
    static bool OnScreen(POINT pt);

private:
    static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM param);
    std::vector<WinInfo> wins_;
};
