#include "Tray.h"

#include <cwchar>

void Tray::Add(HWND owner, HICON icon, UINT callbackMsg)
{
    nid_ = {};
    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = owner;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = callbackMsg;
    nid_.hIcon = icon;
    wcscpy_s(nid_.szTip, L"ClaudePoo");
    added_ = Shell_NotifyIconW(NIM_ADD, &nid_) != FALSE;
}

void Tray::Remove()
{
    if (added_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        added_ = false;
    }
}

void Tray::ShowMenu(HWND owner, const Settings& settings, bool crabVisible) const
{
    HMENU freq = CreatePopupMenu();
    AppendMenuW(freq, MF_STRING, kCmdRare, L"Rare (5-15 min)");
    AppendMenuW(freq, MF_STRING, kCmdNormal, L"Normal (1-5 min)");
    AppendMenuW(freq, MF_STRING, kCmdOften, L"Often (20-60 s)");
    CheckMenuRadioItem(freq, kCmdRare, kCmdOften, kCmdRare + static_cast<UINT>(settings.frequency), MF_BYCOMMAND);

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kCmdShow, L"Show now");
    AppendMenuW(menu, MF_STRING | (crabVisible ? 0u : MF_GRAYED), kCmdHide, L"Hide");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(freq), L"Frequency");
    AppendMenuW(menu, MF_STRING | (settings.paused ? MF_CHECKED : 0u), kCmdPause, L"Pause");
    AppendMenuW(menu, MF_STRING | (IsAutostart() ? MF_CHECKED : 0u), kCmdAutostart, L"Start with Windows");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdQuit, L"Quit");

    POINT pt{};
    GetCursorPos(&pt);
    SetForegroundWindow(owner);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, owner, nullptr);
    PostMessageW(owner, WM_NULL, 0, 0);
    DestroyMenu(menu);
}
