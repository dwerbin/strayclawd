#include <windows.h>

#include "Mate.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\ClaudePooSingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND other = FindWindowW(Mate::kTrayClass, nullptr)) {
            PostMessageW(other, Mate::kMsgShowNow, 0, 0);
        }
        if (mutex) {
            CloseHandle(mutex);
        }
        return 0;
    }

    Mate mate;
    if (!mate.Create(instance)) {
        return 1;
    }
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (mutex) {
        CloseHandle(mutex);
    }
    return 0;
}
