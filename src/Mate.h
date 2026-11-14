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
    static constexpr UINT kMsgTray = WM_APP + 1;
    static constexpr UINT kMsgShowNow = WM_APP + 2;

    bool Create(HINSTANCE instance);

private:
    enum class State { Hidden, SidePeek, Peek, Emerge, Active, Retreat, Falling, Dragged, Land, Parachute };
    enum class Act { Sit, Typing, Walk, Wave, Look, Dance, Sleep, Think, Sneeze, React,
                     Build, Paint, Magic, Cook, Stare, Dizzy,
                     Fish, Read, Juggle, Water, Fitness, Skate, Umbrella, Coffee, Yawn, Chase,
                     Wizard, GreyWizard };
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
    void ApplySeason(double dt);
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
};
