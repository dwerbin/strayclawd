#pragma once

#include <windows.h>
#include <shellapi.h>

#include "Settings.h"

enum TrayCommand : UINT {
    kCmdShow = 1001,
    kCmdHide = 1002,
    kCmdRare = 1010,
    kCmdNormal = 1011,
    kCmdOften = 1012,
    kCmdPause = 1020,
    kCmdAutostart = 1021,
    kCmdFollowTyping = 1022,
    kCmdPartyToday = 1023,
    kCmdConfigure = 1024,
    kCmdQuit = 1099,
};

/** Notification area icon and its context menu. */
class Tray {
public:
    void Add(HWND owner, HICON icon, UINT callbackMsg);
    void Remove();
    void ShowMenu(HWND owner, const Settings& settings, bool crabVisible) const;

private:
    NOTIFYICONDATAW nid_{};
    bool added_ = false;
};
