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
    wcscpy_s(nid_.szTip, L"StrayClawd");
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

    HMENU play = CreatePopupMenu();
    AppendMenuW(play, MF_STRING, kCmdPlayPeek, L"Peek");
    AppendMenuW(play, MF_STRING, kCmdPlayParachute, L"Parachute");
    AppendMenuW(play, MF_STRING, kCmdPlayBuild, L"DIY on a ladder");
    AppendMenuW(play, MF_STRING, kCmdPlayPaint, L"Paint");
    AppendMenuW(play, MF_STRING, kCmdPlayMagic, L"Magic");
    AppendMenuW(play, MF_STRING, kCmdPlayCook, L"Cook");
    AppendMenuW(play, MF_STRING, kCmdPlayDizzy, L"Dizzy");
    AppendMenuW(play, MF_STRING, kCmdPlayFish, L"Fishing");
    AppendMenuW(play, MF_STRING, kCmdPlayRead, L"Reading");
    AppendMenuW(play, MF_STRING, kCmdPlayJuggle, L"Juggling");
    AppendMenuW(play, MF_STRING, kCmdPlayWater, L"Watering the plant");
    AppendMenuW(play, MF_STRING, kCmdPlayFitness, L"Fitness");
    AppendMenuW(play, MF_STRING, kCmdPlaySkate, L"Skateboard");
    AppendMenuW(play, MF_STRING, kCmdPlayUmbrella, L"Rain and rainbow");
    AppendMenuW(play, MF_STRING, kCmdPlayCoffee, L"Coffee");
    AppendMenuW(play, MF_STRING, kCmdPlayYawn, L"Yawn");

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kCmdShow, L"Show now");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(play), L"Play");
    AppendMenuW(menu, MF_STRING | (crabVisible ? 0u : MF_GRAYED), kCmdHide, L"Hide");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(freq), L"Frequency");
    AppendMenuW(menu, MF_STRING | (settings.paused ? MF_CHECKED : 0u), kCmdPause, L"Pause");
    AppendMenuW(menu, MF_STRING | (settings.followTyping ? MF_CHECKED : 0u), kCmdFollowTyping,
                L"Type along with me (counts key presses only)");
    SYSTEMTIME now{};
    GetLocalTime(&now);
    const bool partyToday = settings.partyDate == static_cast<unsigned long>(now.wMonth * 100 + now.wDay);
    AppendMenuW(menu, MF_STRING | (partyToday ? MF_CHECKED : 0u), kCmdPartyToday, L"Today is a party day");
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
