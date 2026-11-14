#include "Settings.h"

#include <windows.h>
#include <string>

namespace {

constexpr wchar_t kKey[] = L"Software\\StrayClawd";
constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kRunValue[] = L"StrayClawd";

DWORD ReadDword(const wchar_t* name, DWORD fallback)
{
    DWORD value = 0;
    DWORD size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER, kKey, name, RRF_RT_REG_DWORD, nullptr, &value, &size) != ERROR_SUCCESS) {
        return fallback;
    }
    return value;
}

void WriteDword(const wchar_t* name, DWORD value)
{
    RegSetKeyValueW(HKEY_CURRENT_USER, kKey, name, REG_DWORD, &value, sizeof(value));
}

}

void Settings::Load()
{
    const DWORD f = ReadDword(L"Frequency", 1);
    frequency = f <= 2 ? static_cast<Frequency>(f) : Frequency::Normal;
    paused = ReadDword(L"Paused", 0) != 0;
    followTyping = ReadDword(L"FollowTyping", 0) != 0;
    partyDate = ReadDword(L"PartyDate", 0);
    plantDay = ReadDword(L"PlantDay", 0);
    plantLevel = ReadDword(L"PlantLevel", 0);
}

void Settings::Save() const
{
    WriteDword(L"Frequency", static_cast<DWORD>(frequency));
    WriteDword(L"Paused", paused ? 1 : 0);
    WriteDword(L"FollowTyping", followTyping ? 1 : 0);
    WriteDword(L"PartyDate", partyDate);
    WriteDword(L"PlantDay", plantDay);
    WriteDword(L"PlantLevel", plantLevel);
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
