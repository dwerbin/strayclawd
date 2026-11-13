#pragma once

enum class Frequency { Rare = 0, Normal = 1, Often = 2 };

/** User settings stored in HKCU\Software\ClaudePoo. */
struct Settings {
    Frequency frequency = Frequency::Normal;
    bool paused = false;

    void Load();
    void Save() const;
};

bool IsAutostart();
void SetAutostart(bool on);
