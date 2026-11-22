#pragma once

#include <windows.h>
#include <cstdint>
#include <random>
#include <vector>

#include "Canvas.h"
#include "Scanner.h"
#include "Settings.h"
#include "Tray.h"

/** The crab: its layered window, its tray icon and its behavior. */
class Mate {
public:
    static constexpr const wchar_t* kCrabClass = L"StrayClawdCrab";
    static constexpr const wchar_t* kTrayClass = L"StrayClawdTray";
    static constexpr const wchar_t* kDuoProp = L"StrayClawdDuo";
    static constexpr UINT kMsgTray = WM_APP + 1;
    static constexpr UINT kMsgShowNow = WM_APP + 2;
    static constexpr UINT kMsgDuoAsk = WM_APP + 10;
    static constexpr UINT kMsgDuoAccept = WM_APP + 11;
    static constexpr UINT kMsgDuoDecline = WM_APP + 12;
    static constexpr UINT kMsgDuoEnd = WM_APP + 13;
    static constexpr UINT kMsgReload = WM_APP + 14;

    bool Create(HINSTANCE instance);

private:
    enum class State { Hidden, SidePeek, Peek, Emerge, Active, Retreat, Falling, Dragged, Land, Parachute };
    enum class Act { Sit, Typing, Walk, Wave, Look, Dance, Sleep, Think, Sneeze, React,
                     Build, Paint, Magic, Cook, Stare, Dizzy,
                     Fish, Read, Juggle, Water, Fitness, Skate, Umbrella, Coffee, Yawn, Chase,
                     Wizard, GreyWizard, Rocket, Dust, Sweep, Yoga, Portal, Climb, Duo };
    enum class Duo { Look, Wave, Talk, Juggle, Cook, Rocket, Bomb, Toast, Scheme, Saber, Boost, Dance, Pray, HighFive,
                     Tug, Pong, Rps, Selfie, Pair, Pillow, Leapfrog, Mirror, Picnic, Chess, Seesaw, Deploy,
                     Pyramid, Band, Standup, Ritual, Review, Conga };
    enum class Surface { None, Window, Ground };
    enum class ZMode { Free, Above, Behind };

    struct Particle {
        char ch;
        double x, y, vx, vy, life;
        uint32_t color;
        double ay;
        double alpha;
    };

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void OnCommand(UINT id);
    void Shutdown();

    void Tick(double dt);
    void StepState(double dt);
    void StepActivity(double dt);
    void Render();
    void UpdateZ(bool force);
    void SetZ(ZMode mode);
    void SetScale(int scale);
    int ScaleAt(POINT pt) const;

    void Appear(bool allowSide);
    void Show();
    void Hide(double delay = -1.0);
    bool PickTopSpot(HWND only);
    bool PickSideSpot();
    void PickGroundSpot();
    bool FollowSurface();
    bool TryLand(double prevY);

    void Enter(State s);
    void StartActivities();
    void NextActivity();
    void BeginAct(Act a);
    void BeginReact(int kind);
    void StartRetreat(bool fast);
    void StartFall(double vx, double vy);
    void OnClick();
    void ShowNow();
    void HideNow();
    double NextDelay();

    void StartParachute();
    void PlayAct(Act a);
    void CheckStare(double dt);
    void StepBuild(double dt);
    void StepPaint(double dt);
    void StepMagic(double dt);
    void StepWizard(double dt);
    void StepGreyWizard(double dt);
    void StepRocket(double dt);
    void StepDust(double dt);
    void StepSweep(double dt);
    void StepYoga(double dt);
    void StepCook(double dt);
    void StepFish(double dt);
    void StepRead(double dt);
    void StepJuggle(double dt);
    void StepWater(double dt);
    void StepFitness(double dt);
    void StepSkate(double dt);
    void StepUmbrella(double dt);
    void StepCoffee(double dt);
    void StepYawn(double dt);
    void StepChase(double dt);
    void StepPortal(double dt);
    void StepClimb(double dt);
    bool FindPortalTarget();
    bool FindClimbTarget();
    void MoveToWindow(double screenX);
    void TestAnimation(int id);
    void ApplySeason(double dt);
    static bool IsCrab(HWND hwnd);
    bool PeerFoot(HWND peer, double* x, double* y) const;
    HWND FindPeer(double maxDx, double* x, double* y) const;
    int FindPeers(double maxDx, HWND* peers, double* xs) const;
    bool IsTrio() const;
    void StepDuoMore(double u, double dt);
    void StepTrio(double u, double dt);
    void DrawDuoBig(Canvas& c, int dx, int dy, int frame);
    void EndAsk();
    bool FindBoostTarget(double x, double footY, double* top);
    bool TryDuo(int forced);
    bool CanJoinDuo(bool forced) const;
    void OnDuoAsk(HWND from, WPARAM wp);
    void OnDuoAccept(HWND from, int kind);
    void OnDuoDecline(HWND from);
    void OnDuoEnd(HWND from);
    void BeginDuo(Duo kind, bool lead, HWND peer, int meet, int side, double approach, int slot = 0);
    void StepDuo(double dt);
    void StepDuoKind(double u, double dt);
    void RenderDuo(int ox, int oy, bool front);
    void RenderProp(POINT crabOrigin);
    void HideProp();
    int MidX() const;
    void BroadcastReload();
    void CheckTyping();
    void UpdateKeyboardHook();
    static LRESULT CALLBACK KeyboardProc(int code, WPARAM wp, LPARAM lp);
    int SideWithRoom() const;
    void LoadWallpaperThumb();
    void Puff(double x, double y, int count, uint32_t color);

    void Spawn(char ch, double x, double y, double vx, double vy, double life, uint32_t color,
               double ay = 0.0, double alpha = 1.0);
    double Rnd();
    double Rnd(double a, double b);
    int RndI(int a, int b);

    HINSTANCE instance_ = nullptr;
    HWND crab_ = nullptr;
    HWND trayWnd_ = nullptr;
    HICON icon_ = nullptr;
    UINT taskbarCreated_ = 0;
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    HGDIOBJ oldBmp_ = nullptr;
    void* bits_ = nullptr;
    int scale_ = 0;
    LARGE_INTEGER qpcFreq_{};
    LARGE_INTEGER qpcLast_{};

    Canvas canvas_;
    Scanner scanner_;
    Settings settings_;
    Tray tray_;
    std::mt19937 rng_;

    State state_ = State::Hidden;
    double stateT_ = 0.0;
    int stateFlag_ = 0;
    Act act_ = Act::Sit;
    double actT_ = 0.0;
    double actDur_ = 0.0;
    int actsLeft_ = 0;
    int actFlag_ = 0;
    int counter_ = 0;
    int side_ = 0;
    int dir_ = 1;
    int pressedKey_ = -1;
    int reactKind_ = 0;
    double timerA_ = 0.0;
    double timerB_ = 0.0;
    double targetRel_ = 0.0;

    Surface surface_ = Surface::None;
    HWND surfHwnd_ = nullptr;
    RECT surfRect_{};
    double edge_ = 0.0;
    double relX_ = 0.0;
    double relY_ = 0.0;
    double footX_ = 0.0;
    double footY_ = 0.0;
    double vx_ = 0.0;
    double vy_ = 0.0;
    double fallScan_ = 0.0;

    ZMode zmode_ = ZMode::Free;
    bool zTopDone_ = false;

    double hideNext_ = 3.0;
    bool shy_ = false;
    bool fastRetreat_ = false;

    Pose pose_;
    double hop_ = 0.0;
    double hopV_ = 0.0;
    int shake_ = 0;
    int bubble_ = -1;
    double blinkT_ = 3.0;
    double clock_ = 0.0;
    std::vector<Particle> particles_;

    bool mouseDown_ = false;
    bool dragging_ = false;
    POINT downPt_{};
    POINT lastMouse_{};
    double grabDx_ = 0.0;
    double grabDy_ = 0.0;
    double mouseVx_ = 0.0;
    double mouseVy_ = 0.0;

    ULONG_PTR gdiplusToken_ = 0;
    int pendingAct_ = -1;
    int lift_ = 0;
    double liftPos_ = 0.0;
    double liftV_ = 0.0;
    int ladderH_ = 0;
    int rocketLevel_ = 0;
    int rocketShift_ = 0;
    bool rocketRide_ = false;
    int rocketFlame_ = 0;
    double flyY_ = 0.0;
    double overX_ = 0.0;
    double skatePos_ = 0.0;
    bool panicFall_ = false;
    int dustPile_ = 0;
    int yogaMat_ = 0;
    bool easel_ = false;
    int easelSide_ = 1;
    int chuteFold_ = -1;
    double chuteX0_ = 0.0;
    double landT_ = 0.0;
    bool stars_ = false;
    int phase_ = 0;
    uint32_t brushColor_ = 0xE8505B;
    uint32_t painting_[35] = {};
    bool revealed_[35] = {};

    int propSide_ = 1;
    bool juggle_ = false;
    int juggleDrop_ = -1;
    int plantLevel_ = -1;
    int rainbow_ = 0;
    int rodX_ = 0;
    int rodY_ = 0;
    int bobberY_ = -1;
    int catchKind_ = 0;
    double seasonT_ = 0.0;

    static Mate* hookOwner_;
    HHOOK keyHook_ = nullptr;
    int keyPresses_ = 0;
    double keyRate_ = 0.0;
    double lastKeyT_ = -10.0;

    double coverCheck_ = 0.0;
    double coveredT_ = 0.0;
    bool cursorNear_ = false;
    double stareCooldown_ = 5.0;
    double farT_ = 0.0;
    double spinAcc_ = 0.0;
    double spinIdle_ = 0.0;
    double lastAngle_ = 0.0;

    HWND peer_ = nullptr;
    Duo duo_ = Duo::Look;
    bool duoLead_ = false;
    int duoSide_ = 1;
    double duoFrom_ = 0.0;
    double duoTo_ = 0.0;
    double duoApproach_ = 1.0;
    double duoLive_ = 0.0;
    double duoLost_ = 0.0;
    double duoCooldown_ = 8.0;
    bool duoDetached_ = false;
    double boostTop_ = 0.0;
    HWND askPeer_ = nullptr;
    double askT_ = 0.0;
    int askDuo_ = 0;
    int askMeet_ = 0;
    int askSide_ = 1;
    int askApproach_ = 10;
    bool askForced_ = false;
    const char* speech_ = nullptr;
    int speechChars_ = 1000;
    int duoProp_ = 0;
    int duoLevel_ = 0;
    int duoFlame_ = 0;
    int signDrop_ = -1;
    int duoCount_ = 0;
    bool duoBalls_ = false;
    bool duoGone_ = false;
    double duoBoom_ = 0.0;
    HWND peer2_ = nullptr;
    HWND askPeer2_ = nullptr;
    int askAccepts_ = 0;
    int askSlot_ = 0;
    int duoSlot_ = 0;
    double duoMeet_ = 0.0;
    int duoSeed_ = 0;
    bool duoBig_ = false;
    double duoU_ = 0.0;
    int duoCrumb_ = 0;
    int rpsSign_ = -1;
    char bugIcon_ = 0;
    uint32_t bugColor_ = 0;
    int ownProp_ = 0;

    HWND prop_ = nullptr;
    HDC propDc_ = nullptr;
    HBITMAP propDib_ = nullptr;
    HGDIOBJ propOldBmp_ = nullptr;
    void* propBits_ = nullptr;
    int propScale_ = 0;
    bool propShown_ = false;
    bool propTop_ = false;

    bool forcedAct_ = false;
    HWND jumpHwnd_ = nullptr;
    RECT jumpRect_{};
    double jumpX_ = 0.0;
    double jumpFrom_ = 0.0;
    double jumpTo_ = 0.0;
    double jumpWalk_ = 0.0;
    int jumpSide_ = 1;
    int jumpH_ = 0;
    int sink_ = 0;
    int portalX_ = -1;
    double portalOpen_ = 0.0;
    int portalKind_ = 0;
    double portalFarX_ = 0.0;
    double portalFarY_ = 0.0;
    double portalFarOpen_ = 0.0;
    int portalFarKind_ = 0;
    Canvas propCanvas_;
};
