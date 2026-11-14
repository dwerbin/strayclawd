#pragma once

enum class Frequency { Rare = 0, Normal = 1, Often = 2 };

/** User settings stored in HKCU\Software\StrayClawd. */
struct Settings {
    Frequency frequency = Frequency::Normal;
    bool paused = false;
    bool followTyping = false;
    unsigned long partyDate = 0;
    unsigned long plantLevel = 0;

    void Load();
    void Save() const;
};

bool IsAutostart();
void SetAutostart(bool on);
