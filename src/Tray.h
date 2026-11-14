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
    kCmdPlayPeek = 1030,
    kCmdPlayParachute = 1031,
    kCmdPlayBuild = 1032,
    kCmdPlayPaint = 1033,
    kCmdPlayMagic = 1034,
    kCmdPlayCook = 1035,
    kCmdPlayDizzy = 1036,
    kCmdPlayFish = 1037,
    kCmdPlayRead = 1038,
    kCmdPlayJuggle = 1039,
    kCmdPlayWater = 1040,
    kCmdPlayFitness = 1041,
    kCmdPlaySkate = 1042,
    kCmdPlayUmbrella = 1043,
    kCmdPlayCoffee = 1044,
    kCmdPlayYawn = 1045,
    kCmdPlayWizard = 1046,
    kCmdPlayGreyWizard = 1047,
    kCmdFollowTyping = 1022,
    kCmdPartyToday = 1023,
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
