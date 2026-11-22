#include "Settings.h"

#include <windows.h>
#include <algorithm>
#include <string>

const AnimInfo kAnims[kAnimCount] = {
    {L"Typing", L"Typing", AnimGroup::Activity, 5, nullptr},
    {L"Walk", L"Walk", AnimGroup::Activity, 3, nullptr},
    {L"Wave", L"Wave", AnimGroup::Activity, 1, nullptr},
    {L"Look", L"Look at the mouse", AnimGroup::Activity, 2, nullptr},
    {L"Dance", L"Dance", AnimGroup::Activity, 2, nullptr},
    {L"Sleep", L"Sleep", AnimGroup::Activity, 1, L"x4 at night"},
    {L"Think", L"Think", AnimGroup::Activity, 2, nullptr},
    {L"Sneeze", L"Sneeze", AnimGroup::Activity, 1, nullptr},
    {L"Sit", L"Sit", AnimGroup::Activity, 2, nullptr},
    {L"Build", L"DIY on a ladder", AnimGroup::Activity, 1, nullptr},
    {L"Paint", L"Paint", AnimGroup::Activity, 1, nullptr},
    {L"Magic", L"Magic", AnimGroup::Activity, 1, nullptr},
    {L"Cook", L"Cook", AnimGroup::Activity, 1, nullptr},
    {L"Wizard", L"Magic: young wizard", AnimGroup::Activity, 1, nullptr},
    {L"GreyWizard", L"Magic: grey wizard", AnimGroup::Activity, 1, nullptr},
    {L"Rocket", L"Rocket", AnimGroup::Activity, 1, nullptr},
    {L"Dust", L"Dusting", AnimGroup::Activity, 1, nullptr},
    {L"Sweep", L"Sweeping", AnimGroup::Activity, 1, nullptr},
    {L"Yoga", L"Yoga", AnimGroup::Activity, 1, nullptr},
    {L"Fish", L"Fishing", AnimGroup::Activity, 1, nullptr},
    {L"Read", L"Reading", AnimGroup::Activity, 1, nullptr},
    {L"Juggle", L"Juggling", AnimGroup::Activity, 1, nullptr},
    {L"Water", L"Watering the plant", AnimGroup::Activity, 1, nullptr},
    {L"Fitness", L"Fitness", AnimGroup::Activity, 1, nullptr},
    {L"Skate", L"Skateboard", AnimGroup::Activity, 1, nullptr},
    {L"Umbrella", L"Rain and rainbow", AnimGroup::Activity, 1, nullptr},
    {L"Coffee", L"Coffee", AnimGroup::Activity, 3, L"6-11 h only"},
    {L"Yawn", L"Yawn", AnimGroup::Activity, 2, L"13-15 h and evening"},
    {L"DuoLook", L"Look at each other", AnimGroup::Duo, 2, nullptr},
    {L"DuoWave", L"Wave at each other", AnimGroup::Duo, 2, nullptr},
    {L"DuoTalk", L"Talk", AnimGroup::Duo, 2, nullptr},
    {L"DuoJuggle", L"Juggle together", AnimGroup::Duo, 1, nullptr},
    {L"DuoCook", L"Cook together", AnimGroup::Duo, 1, nullptr},
    {L"DuoRocket", L"Build the big rocket", AnimGroup::Duo, 1, nullptr},
    {L"DuoBomb", L"Leaking bomb (this is fine)", AnimGroup::Duo, 1, nullptr},
    {L"DuoToast", L"Toast with mugs", AnimGroup::Duo, 1, nullptr},
    {L"DuoScheme", L"Scheme together", AnimGroup::Duo, 1, nullptr},
    {L"DuoSaber", L"Light saber duel", AnimGroup::Duo, 1, nullptr},
    {L"DuoBoost", L"Boost onto a window", AnimGroup::Duo, 1, L"needs a window above"},
    {L"DuoDance", L"Dance together", AnimGroup::Duo, 1, nullptr},
    {L"DuoPray", L"Pray to Saint Algo", AnimGroup::Duo, 1, nullptr},
    {L"DuoHighFive", L"Jump and high five", AnimGroup::Duo, 2, nullptr},
    {L"DuoTug", L"Tug of war", AnimGroup::Duo, 1, nullptr},
    {L"DuoPong", L"Ping pong", AnimGroup::Duo, 1, nullptr},
    {L"DuoRps", L"Rock paper scissors", AnimGroup::Duo, 2, nullptr},
    {L"DuoSelfie", L"Selfie", AnimGroup::Duo, 1, nullptr},
    {L"DuoPair", L"Pair programming", AnimGroup::Duo, 2, nullptr},
    {L"DuoPillow", L"Pillow fight", AnimGroup::Duo, 1, nullptr},
    {L"DuoLeapfrog", L"Leapfrog", AnimGroup::Duo, 1, L"needs room on the edge"},
    {L"DuoMirror", L"Mirror", AnimGroup::Duo, 1, nullptr},
    {L"DuoPicnic", L"Picnic", AnimGroup::Duo, 1, nullptr},
    {L"DuoChess", L"Chess", AnimGroup::Duo, 1, nullptr},
    {L"DuoSeesaw", L"Seesaw", AnimGroup::Duo, 1, nullptr},
    {L"DuoDeploy", L"Deploy on Friday", AnimGroup::Duo, 1, nullptr},
    {L"TrioPyramid", L"Trio: pyramid", AnimGroup::Duo, 2, L"needs 3 crabs"},
    {L"TrioBand", L"Trio: band", AnimGroup::Duo, 2, L"needs 3 crabs"},
    {L"TrioStandup", L"Trio: stand-up meeting", AnimGroup::Duo, 2, L"needs 3 crabs"},
    {L"TrioRitual", L"Trio: Saint Algo ritual", AnimGroup::Duo, 1, L"needs 3 crabs"},
    {L"TrioReview", L"Trio: code review", AnimGroup::Duo, 2, L"needs 3 crabs"},
    {L"TrioConga", L"Trio: conga line", AnimGroup::Duo, 2, L"needs 3 crabs, room on the edge"},
    {L"Parachute", L"Arrive by parachute", AnimGroup::Event, 15, L"% of the visits"},
    {L"SidePeek", L"Peek from a window side", AnimGroup::Event, 25, L"% of the visits"},
    {L"Shy", L"Shy peek (goes back)", AnimGroup::Event, 30, L"% of the visits"},
    {L"Stare", L"Stare at the cursor", AnimGroup::Event, 30, L"% when the cursor comes"},
    {L"Chase", L"Chase the cursor", AnimGroup::Event, 20, L"% when the cursor comes"},
    {L"DuoChance", L"Duo with a crab nearby", AnimGroup::Event, 40, L"% per activity"},
};

namespace {

constexpr wchar_t kKey[] = L"Software\\StrayClawd";
constexpr wchar_t kAnimKey[] = L"Software\\StrayClawd\\Animations";
constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kRunValue[] = L"StrayClawd";
constexpr DWORD kDisabledBit = 0x10000;

DWORD ReadDword(const wchar_t* key, const wchar_t* name, DWORD fallback)
{
    DWORD value = 0;
    DWORD size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER, key, name, RRF_RT_REG_DWORD, nullptr, &value, &size) != ERROR_SUCCESS) {
        return fallback;
    }
    return value;
}

void WriteDword(const wchar_t* key, const wchar_t* name, DWORD value)
{
    RegSetKeyValueW(HKEY_CURRENT_USER, key, name, REG_DWORD, &value, sizeof(value));
}

}

int MaxWeight(AnimGroup group)
{
    return group == AnimGroup::Event ? 100 : 20;
}

Settings::Settings()
{
    ResetAnimations();
}

void Settings::ResetAnimations()
{
    for (int i = 0; i < kAnimCount; ++i) {
        weight[i] = kAnims[i].def;
        enabled[i] = true;
    }
}

void Settings::Load()
{
    const DWORD f = ReadDword(kKey, L"Frequency", 1);
    frequency = f <= 2 ? static_cast<Frequency>(f) : Frequency::Normal;
    paused = ReadDword(kKey, L"Paused", 0) != 0;
    followTyping = ReadDword(kKey, L"FollowTyping", 0) != 0;
    partyDate = ReadDword(kKey, L"PartyDate", 0);
    plantLevel = ReadDword(kKey, L"PlantLevel", 0);
    for (int i = 0; i < kAnimCount; ++i) {
        const DWORD v = ReadDword(kAnimKey, kAnims[i].key, static_cast<DWORD>(kAnims[i].def));
        weight[i] = std::min(static_cast<int>(v & 0xFFFF), MaxWeight(kAnims[i].group));
        enabled[i] = (v & kDisabledBit) == 0;
    }
}

void Settings::Save() const
{
    WriteDword(kKey, L"Frequency", static_cast<DWORD>(frequency));
    WriteDword(kKey, L"Paused", paused ? 1 : 0);
    WriteDword(kKey, L"FollowTyping", followTyping ? 1 : 0);
    WriteDword(kKey, L"PartyDate", partyDate);
    WriteDword(kKey, L"PlantLevel", plantLevel);
}

void Settings::SaveAnimations() const
{
    for (int i = 0; i < kAnimCount; ++i) {
        WriteDword(kAnimKey, kAnims[i].key, static_cast<DWORD>(weight[i]) | (enabled[i] ? 0 : kDisabledBit));
    }
}

bool IsAutostart()
{
    DWORD size = 0;
    return RegGetValueW(HKEY_CURRENT_USER, kRunKey, kRunValue, RRF_RT_REG_SZ, nullptr, nullptr, &size) == ERROR_SUCCESS;
}

void SetAutostart(bool on)
{
    if (!on) {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kRunKey, kRunValue);
        return;
    }
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    const std::wstring value = L"\"" + std::wstring(path) + L"\"";
    RegSetKeyValueW(HKEY_CURRENT_USER, kRunKey, kRunValue, REG_SZ, value.c_str(),
                    static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
}
