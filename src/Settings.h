#pragma once

enum class Frequency { Rare = 0, Normal = 1, Often = 2 };

enum class AnimGroup { Activity, Duo, Event };

enum AnimId : int {
    kAnimTyping,
    kAnimWalk,
    kAnimWave,
    kAnimLook,
    kAnimDance,
    kAnimSleep,
    kAnimThink,
    kAnimSneeze,
    kAnimSit,
    kAnimBuild,
    kAnimPaint,
    kAnimMagic,
    kAnimCook,
    kAnimWizard,
    kAnimGreyWizard,
    kAnimRocket,
    kAnimDust,
    kAnimSweep,
    kAnimYoga,
    kAnimFish,
    kAnimRead,
    kAnimJuggle,
    kAnimWater,
    kAnimFitness,
    kAnimSkate,
    kAnimUmbrella,
    kAnimCoffee,
    kAnimYawn,
    kAnimDuoLook,
    kAnimDuoWave,
    kAnimDuoTalk,
    kAnimDuoJuggle,
    kAnimDuoCook,
    kAnimDuoRocket,
    kAnimDuoBomb,
    kAnimDuoToast,
    kAnimDuoScheme,
    kAnimDuoSaber,
    kAnimDuoBoost,
    kAnimDuoDance,
    kAnimDuoPray,
    kAnimDuoHighFive,
    kAnimDuoTug,
    kAnimDuoPong,
    kAnimDuoRps,
    kAnimDuoSelfie,
    kAnimDuoPair,
    kAnimDuoPillow,
    kAnimDuoLeapfrog,
    kAnimDuoMirror,
    kAnimDuoPicnic,
    kAnimDuoChess,
    kAnimDuoSeesaw,
    kAnimDuoDeploy,
    kAnimTrioPyramid,
    kAnimTrioBand,
    kAnimTrioStandup,
    kAnimTrioRitual,
    kAnimTrioReview,
    kAnimTrioConga,
    kAnimParachute,
    kAnimSidePeek,
    kAnimShy,
    kAnimStare,
    kAnimChase,
    kAnimDuoChance,
    kAnimCount
};

constexpr int kAnimFirstDuo = kAnimDuoLook;
constexpr int kDuoCount = kAnimTrioConga - kAnimDuoLook + 1;
constexpr int kFirstTrio = kAnimTrioPyramid - kAnimDuoLook;

/** One configurable animation: registry key, label, group, default weight and an optional note. */
struct AnimInfo {
    const wchar_t* key;
    const wchar_t* label;
    AnimGroup group;
    int def;
    const wchar_t* note;
};

extern const AnimInfo kAnims[kAnimCount];

/** Highest weight of a group: a relative weight for activities and duos, a percent for events. */
int MaxWeight(AnimGroup group);

/** User settings stored in HKCU\Software\StrayClawd. */
struct Settings {
    Frequency frequency = Frequency::Normal;
    bool paused = false;
    bool followTyping = false;
    unsigned long partyDate = 0;
    unsigned long plantLevel = 0;
    int weight[kAnimCount] = {};
    bool enabled[kAnimCount] = {};

    Settings();
    int Weight(int id) const { return enabled[id] ? weight[id] : 0; }
    void ResetAnimations();
    void Load();
    void Save() const;
    void SaveAnimations() const;
};

bool IsAutostart();
void SetAutostart(bool on);
