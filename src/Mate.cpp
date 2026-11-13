#include "Mate.h"

#include <shellscalingapi.h>

#include <algorithm>
#include <cmath>
#include <cstring>

#include <objidl.h>
namespace Gdiplus {
using std::max;
using std::min;
}
#include <gdiplus.h>

namespace {

constexpr UINT_PTR kTimerId = 1;
constexpr int kCanvasW = 56;
constexpr int kCanvasH = 48;
constexpr int kCrabX = 20;
constexpr int kCrabY = kCanvasH - kCrabH;
constexpr int kFootLX = kCrabX + kCrabW / 2;

constexpr uint32_t kWhite = 0xFFFFFF;
constexpr uint32_t kHeart = 0xE8505B;
constexpr uint32_t kNote = 0xF2C14E;
constexpr uint32_t kDrop = 0x6EC6FF;
constexpr uint32_t kBang = 0xFFD84A;
constexpr uint32_t kCode = 0xB8F5A0;

constexpr uint32_t kSteam = 0xF0F0F0;
constexpr uint32_t kCarrot = 0xF08A24;
constexpr uint32_t kPea = 0x5DBB4A;
constexpr uint32_t kTomato = 0xE23B3B;
constexpr double kPi = 3.14159265358979323846;

constexpr char kCodeGlyphs[] = "{};<>/=01";

double Smooth(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double Lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

int Sign(double v)
{
    return (v > 0.0) - (v < 0.0);
}

}

bool Mate::Create(HINSTANCE instance)
{
    instance_ = instance;
    rng_.seed(std::random_device{}());
    Gdiplus::GdiplusStartupInput gdiplusInput;
    Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusInput, nullptr);
    settings_.Load();
    icon_ = CreateCrabIcon(GetSystemMetrics(SM_CXSMICON));

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &Mate::WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_HAND);
    wc.hIcon = icon_;
    wc.hIconSm = icon_;
    wc.lpszClassName = kCrabClass;
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    wc.lpszClassName = kTrayClass;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (!RegisterClassExW(&wc)) {
        return false;
    }

    trayWnd_ = CreateWindowExW(0, kTrayClass, L"ClaudePoo", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, this);
    crab_ = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kCrabClass, L"ClaudePoo", WS_POPUP,
                            0, 0, 1, 1, nullptr, nullptr, instance, this);
    if (!trayWnd_ || !crab_) {
        return false;
    }

    POINT origin{0, 0};
    SetScale(ScaleAt(origin));
    taskbarCreated_ = RegisterWindowMessageW(L"TaskbarCreated");
    tray_.Add(trayWnd_, icon_, kMsgTray);

    QueryPerformanceFrequency(&qpcFreq_);
    QueryPerformanceCounter(&qpcLast_);
    hideNext_ = 3.0;
    SetTimer(crab_, kTimerId, 33, nullptr);
    return true;
}

LRESULT CALLBACK Mate::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
    }
    auto* self = reinterpret_cast<Mate*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self) {
        return self->OnMessage(hwnd, msg, wp, lp);
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT Mate::OnMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == taskbarCreated_ && taskbarCreated_ != 0) {
        if (hwnd == trayWnd_) {
            tray_.Add(trayWnd_, icon_, kMsgTray);
        }
        return 0;
    }

    switch (msg) {
    case WM_TIMER: {
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        double dt = static_cast<double>(now.QuadPart - qpcLast_.QuadPart) / static_cast<double>(qpcFreq_.QuadPart);
        qpcLast_ = now;
        Tick(std::clamp(dt, 0.0, 0.1));
        return 0;
    }
    case kMsgTray:
        switch (LOWORD(lp)) {
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
            tray_.ShowMenu(trayWnd_, settings_, state_ != State::Hidden);
            break;
        case WM_LBUTTONDBLCLK:
            ShowNow();
            break;
        default:
            break;
        }
        return 0;
    case kMsgShowNow:
        ShowNow();
        return 0;
    case WM_COMMAND:
        OnCommand(LOWORD(wp));
        return 0;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    case WM_SETCURSOR:
        if (hwnd == crab_) {
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return TRUE;
        }
        break;
    case WM_LBUTTONDOWN:
        if (hwnd == crab_ && state_ != State::Hidden) {
            SetCapture(crab_);
            mouseDown_ = true;
            dragging_ = false;
            GetCursorPos(&downPt_);
            lastMouse_ = downPt_;
            grabDx_ = downPt_.x - footX_;
            grabDy_ = downPt_.y - footY_;
        }
        return 0;
    case WM_MOUSEMOVE:
        if (mouseDown_ && !dragging_) {
            POINT c{};
            GetCursorPos(&c);
            if (std::abs(c.x - downPt_.x) > 4 || std::abs(c.y - downPt_.y) > 4) {
                dragging_ = true;
                bubble_ = -1;
                surface_ = Surface::None;
                surfHwnd_ = nullptr;
                side_ = 0;
                mouseVx_ = 0.0;
                mouseVy_ = 0.0;
                SetZ(ZMode::Free);
                Enter(State::Dragged);
            }
        }
        return 0;
    case WM_LBUTTONUP:
        if (mouseDown_) {
            const bool wasDrag = dragging_;
            mouseDown_ = false;
            dragging_ = false;
            ReleaseCapture();
            if (wasDrag) {
                StartFall(std::clamp(mouseVx_, -2000.0, 2000.0) * 0.5, std::clamp(mouseVy_ * 0.5, -1500.0, 0.0));
            } else {
                OnClick();
            }
        }
        return 0;
    case WM_CAPTURECHANGED:
        if (dragging_) {
            dragging_ = false;
            mouseDown_ = false;
            StartFall(0.0, 0.0);
        }
        mouseDown_ = false;
        return 0;
    case WM_RBUTTONUP:
        if (hwnd == crab_) {
            tray_.ShowMenu(trayWnd_, settings_, state_ != State::Hidden);
        }
        return 0;
    case WM_DESTROY:
        if (hwnd == trayWnd_) {
            PostQuitMessage(0);
        }
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void Mate::OnCommand(UINT id)
{
    switch (id) {
    case kCmdShow:
        ShowNow();
        break;
    case kCmdHide:
        HideNow();
        break;
    case kCmdRare:
    case kCmdNormal:
    case kCmdOften:
        settings_.frequency = static_cast<Frequency>(id - kCmdRare);
        settings_.Save();
        if (state_ == State::Hidden) {
            hideNext_ = std::min(hideNext_, NextDelay());
        }
        break;
    case kCmdPause:
        settings_.paused = !settings_.paused;
        settings_.Save();
        if (settings_.paused) {
            HideNow();
        }
        break;
    case kCmdAutostart:
        SetAutostart(!IsAutostart());
        break;
    case kCmdQuit:
        Shutdown();
        break;
    case kCmdPlayPeek:
        if (state_ == State::Hidden) {
            Appear(false);
        }
        break;
    case kCmdPlayParachute:
        if (state_ != State::Parachute && state_ != State::Dragged) {
            if (state_ != State::Hidden) {
                Hide();
            }
            StartParachute();
        }
        break;
    case kCmdPlayBuild: PlayAct(Act::Build); break;
    case kCmdPlayPaint: PlayAct(Act::Paint); break;
    case kCmdPlayMagic: PlayAct(Act::Magic); break;
    case kCmdPlayCook: PlayAct(Act::Cook); break;
    case kCmdPlayDizzy: PlayAct(Act::Dizzy); break;
    default:
        break;
    }
}

void Mate::PlayAct(Act a)
{
    if (state_ == State::Active) {
        BeginAct(a);
    } else {
        pendingAct_ = static_cast<int>(a);
        if (state_ == State::Hidden) {
            Appear(false);
        }
    }
}

void Mate::Shutdown()
{
    KillTimer(crab_, kTimerId);
    tray_.Remove();
    DestroyWindow(crab_);
    crab_ = nullptr;
    if (memDc_) {
        SelectObject(memDc_, oldBmp_);
        DeleteObject(dib_);
        DeleteDC(memDc_);
        memDc_ = nullptr;
    }
    if (icon_) {
        DestroyIcon(icon_);
        icon_ = nullptr;
    }
    if (gdiplusToken_) {
        Gdiplus::GdiplusShutdown(gdiplusToken_);
        gdiplusToken_ = 0;
    }
    DestroyWindow(trayWnd_);
}

void Mate::ShowNow()
{
    if (state_ == State::Hidden) {
        Appear(false);
    } else if (state_ == State::Active) {
        BeginReact(0);
    }
}

void Mate::HideNow()
{
    if (state_ == State::Active) {
        StartRetreat(true);
    } else if (state_ != State::Hidden && state_ != State::Retreat) {
        Hide();
    }
}

double Mate::NextDelay()
{
    switch (settings_.frequency) {
    case Frequency::Rare: return Rnd(300.0, 900.0);
    case Frequency::Often: return Rnd(20.0, 60.0);
    default: return Rnd(60.0, 300.0);
    }
}

double Mate::Rnd()
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng_);
}

double Mate::Rnd(double a, double b)
{
    return a + (b - a) * Rnd();
}

int Mate::RndI(int a, int b)
{
    return std::uniform_int_distribution<int>(a, b)(rng_);
}

int Mate::ScaleAt(POINT pt) const
{
    HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    UINT dx = 96;
    UINT dy = 96;
    if (FAILED(GetDpiForMonitor(mon, MDT_EFFECTIVE_DPI, &dx, &dy))) {
        dx = 96;
    }
    return std::max(2, static_cast<int>(std::lround(4.0 * dx / 96.0)));
}

void Mate::SetScale(int scale)
{
    if (scale == scale_ && memDc_) {
        return;
    }
    scale_ = scale;
    canvas_.Resize(kCanvasW, kCanvasH, scale_);
    if (!memDc_) {
        memDc_ = CreateCompatibleDC(nullptr);
    } else {
        SelectObject(memDc_, oldBmp_);
        DeleteObject(dib_);
    }
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = canvas_.PixelWidth();
    bi.bmiHeader.biHeight = -canvas_.PixelHeight();
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    dib_ = CreateDIBSection(memDc_, &bi, DIB_RGB_COLORS, &bits_, nullptr, 0);
    oldBmp_ = SelectObject(memDc_, dib_);
}

void Mate::Spawn(char ch, double x, double y, double vx, double vy, double life, uint32_t color, double ay, double alpha)
{
    if (particles_.size() < 60) {
        particles_.push_back({ch, x, y, vx, vy, life, color, ay, alpha});
    }
}

void Mate::Puff(double x, double y, int count, uint32_t color)
{
    for (int i = 0; i < count; ++i) {
        const double a = 2.0 * kPi * i / count + Rnd(-0.3, 0.3);
        Spawn('*', x - 1, y - 1, std::cos(a) * 14.0, std::sin(a) * 9.0, 0.45, color);
    }
}

void Mate::Enter(State s)
{
    state_ = s;
    stateT_ = 0.0;
    stateFlag_ = 0;
    timerA_ = 0.0;
}

void Mate::Appear(bool allowSide)
{
    scanner_.Refresh();
    particles_.clear();
    bubble_ = -1;
    hop_ = 0.0;
    hopV_ = 0.0;
    fastRetreat_ = false;
    if (allowSide && Rnd() < 0.15) {
        StartParachute();
        return;
    }
    if (allowSide && Rnd() < 0.25 && PickSideSpot()) {
        Enter(State::SidePeek);
        Show();
        return;
    }
    if (!PickTopSpot(nullptr)) {
        PickGroundSpot();
    }
    shy_ = allowSide && Rnd() < 0.3;
    Enter(State::Peek);
    Show();
}

void Mate::Show()
{
    pose_ = Pose{};
    StepState(0.0);
    if (state_ == State::Hidden) {
        return;
    }
    zTopDone_ = false;
    UpdateZ(true);
    Render();
    ShowWindow(crab_, SW_SHOWNA);
    UpdateZ(true);
}

void Mate::Hide(double delay)
{
    ShowWindow(crab_, SW_HIDE);
    if (mouseDown_) {
        mouseDown_ = false;
        dragging_ = false;
        ReleaseCapture();
    }
    state_ = State::Hidden;
    stateT_ = 0.0;
    particles_.clear();
    bubble_ = -1;
    surface_ = Surface::None;
    surfHwnd_ = nullptr;
    side_ = 0;
    zmode_ = ZMode::Free;
    fastRetreat_ = false;
    hideNext_ = delay > 0.0 ? delay : NextDelay();
}

void Mate::StartParachute()
{
    std::vector<HMONITOR> mons;
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR m, HDC, LPRECT, LPARAM lp) -> BOOL {
        reinterpret_cast<std::vector<HMONITOR>*>(lp)->push_back(m);
        return TRUE;
    }, reinterpret_cast<LPARAM>(&mons));
    if (mons.empty()) {
        return;
    }
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(mons[static_cast<size_t>(RndI(0, static_cast<int>(mons.size()) - 1))], &mi);
    const RECT wa = mi.rcWork;
    POINT center{(wa.left + wa.right) / 2, (wa.top + wa.bottom) / 2};
    SetScale(ScaleAt(center));
    const double s = scale_;
    particles_.clear();
    bubble_ = -1;
    hop_ = 0.0;
    hopV_ = 0.0;
    fastRetreat_ = false;
    surface_ = Surface::Ground;
    surfHwnd_ = nullptr;
    surfRect_ = wa;
    side_ = 0;
    chuteX0_ = Rnd(wa.left + 30.0 * s, std::max(wa.left + 30.0 * s, wa.right - 30.0 * s));
    footX_ = chuteX0_;
    footY_ = mi.rcMonitor.top - 2.0 * s;
    SetZ(ZMode::Free);
    Enter(State::Parachute);
    Show();
}

bool Mate::PickTopSpot(HWND only)
{
    const auto& wins = scanner_.Windows();
    std::vector<size_t> order;
    for (size_t i = 0; i < wins.size(); ++i) {
        if (wins[i].surface && (!only || wins[i].hwnd == only)) {
            order.push_back(i);
        }
    }
    std::shuffle(order.begin(), order.end(), rng_);
    for (size_t idx : order) {
        const RECT& r = wins[idx].rect;
        POINT center{(r.left + r.right) / 2, r.top};
        const int s = ScaleAt(center);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi);
        if (r.top - 12 * s < mi.rcMonitor.top) {
            continue;
        }
        const int lo = r.left + 12 * s;
        const int hi = r.right - 12 * s;
        if (hi <= lo) {
            continue;
        }
        for (int attempt = 0; attempt < 8; ++attempt) {
            const int x = RndI(lo, hi);
            POINT edge{x, r.top + 3};
            POINT a{x - 8 * s, r.top - 6 * s};
            POINT b{x + 8 * s, r.top - 6 * s};
            if (scanner_.IsCovered(edge, idx) || scanner_.IsCovered(a, idx) || scanner_.IsCovered(b, idx)) {
                continue;
            }
            if (!Scanner::OnScreen(a) || !Scanner::OnScreen(b)) {
                continue;
            }
            SetScale(s);
            surface_ = Surface::Window;
            surfHwnd_ = wins[idx].hwnd;
            surfRect_ = r;
            side_ = 0;
            relX_ = x - r.left;
            edge_ = r.top;
            footX_ = x;
            footY_ = r.top + 11.0 * s;
            return true;
        }
    }
    return false;
}

bool Mate::PickSideSpot()
{
    const auto& wins = scanner_.Windows();
    std::vector<size_t> order;
    for (size_t i = 0; i < wins.size(); ++i) {
        if (wins[i].surface) {
            order.push_back(i);
        }
    }
    std::shuffle(order.begin(), order.end(), rng_);
    for (size_t idx : order) {
        const RECT& r = wins[idx].rect;
        POINT center{(r.left + r.right) / 2, (r.top + r.bottom) / 2};
        const int s = ScaleAt(center);
        int side = Rnd() < 0.5 ? -1 : 1;
        for (int attempt = 0; attempt < 2; ++attempt, side = -side) {
            const int y0 = r.top + 16 * s;
            const int y1 = r.bottom - 4 * s;
            if (y1 <= y0) {
                break;
            }
            const int fy = RndI(y0, y1);
            const int ex = side < 0 ? r.left : r.right;
            POINT out{ex + side * 4 * s, fy - 5 * s};
            POINT in{ex - side * 3, fy - 5 * s};
            if (scanner_.IsCovered(out, idx) || scanner_.IsCovered(in, idx) || !Scanner::OnScreen(out)) {
                continue;
            }
            SetScale(s);
            surface_ = Surface::Window;
            surfHwnd_ = wins[idx].hwnd;
            surfRect_ = r;
            side_ = side;
            relY_ = fy - r.top;
            footX_ = ex - side * 9.0 * s;
            footY_ = fy;
            return true;
        }
    }
    return false;
}

void Mate::PickGroundSpot()
{
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    POINT origin{0, 0};
    GetMonitorInfoW(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &mi);
    const RECT wa = mi.rcWork;
    POINT corner{wa.left + 1, wa.bottom - 1};
    SetScale(ScaleAt(corner));
    surface_ = Surface::Ground;
    surfHwnd_ = nullptr;
    surfRect_ = wa;
    side_ = 0;
    relX_ = Rnd(20.0 * scale_, (wa.right - wa.left) - 20.0 * scale_);
    edge_ = wa.bottom;
    footX_ = wa.left + relX_;
    footY_ = wa.bottom + 11.0 * scale_;
}

bool Mate::FollowSurface()
{
    if (surface_ == Surface::Window) {
        if (!Scanner::IsAlive(surfHwnd_)) {
            return false;
        }
        RECT r{};
        if (!Scanner::GetFrame(surfHwnd_, &r)) {
            return false;
        }
        surfRect_ = r;
        if (side_ == 0) {
            MONITORINFO mi{};
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi);
            if (r.top - kCrabH * scale_ < mi.rcMonitor.top) {
                return false;
            }
        }
        edge_ = r.top;
    } else if (surface_ == Surface::Ground) {
        edge_ = surfRect_.bottom;
    } else {
        return false;
    }
    if (side_ == 0) {
        const double w = surfRect_.right - surfRect_.left;
        const double m = 10.0 * scale_;
        relX_ = std::clamp(relX_, m, std::max(m, w - m));
    }
    return true;
}

bool Mate::TryLand(double prevY)
{
    const double s = scale_;
    const auto& wins = scanner_.Windows();
    for (size_t i = 0; i < wins.size(); ++i) {
        if (!wins[i].surface) {
            continue;
        }
        const RECT& r = wins[i].rect;
        if (prevY > r.top || footY_ < r.top || footX_ < r.left + 4 * s || footX_ > r.right - 4 * s) {
            continue;
        }
        POINT p{static_cast<LONG>(footX_), r.top + 2};
        if (scanner_.IsCovered(p, i)) {
            continue;
        }
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi);
        if (r.top - kCrabH * s < mi.rcMonitor.top) {
            continue;
        }
        surface_ = Surface::Window;
        surfHwnd_ = wins[i].hwnd;
        surfRect_ = r;
        side_ = 0;
        relX_ = footX_ - r.left;
        edge_ = r.top;
        footY_ = edge_;
        Enter(State::Land);
        return true;
    }

    POINT probe{static_cast<LONG>(footX_), static_cast<LONG>(prevY)};
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(MonitorFromPoint(probe, MONITOR_DEFAULTTONEAREST), &mi);
    if (footY_ >= mi.rcWork.bottom) {
        surface_ = Surface::Ground;
        surfHwnd_ = nullptr;
        surfRect_ = mi.rcWork;
        side_ = 0;
        relX_ = footX_ - mi.rcWork.left;
        edge_ = mi.rcWork.bottom;
        footY_ = edge_;
        Enter(State::Land);
        return true;
    }
    return false;
}

void Mate::StartActivities()
{
    actsLeft_ = RndI(4, 8);
    Enter(State::Active);
    if (pendingAct_ >= 0) {
        NextActivity();
    } else {
        BeginAct(Act::Typing);
    }
}

void Mate::NextActivity()
{
    if (pendingAct_ >= 0) {
        const Act a = static_cast<Act>(pendingAct_);
        pendingAct_ = -1;
        BeginAct(a);
        return;
    }
    if (actsLeft_-- <= 0) {
        StartRetreat(false);
        return;
    }
    struct Weight {
        Act act;
        int weight;
    };
    static const Weight kTable[] = {
        {Act::Typing, 5}, {Act::Walk, 3}, {Act::Wave, 1}, {Act::Look, 2}, {Act::Dance, 2},
        {Act::Sleep, 1},  {Act::Think, 2}, {Act::Sneeze, 1}, {Act::Sit, 2},
        {Act::Build, 1},  {Act::Paint, 1}, {Act::Magic, 1}, {Act::Cook, 1},
    };
    int total = 0;
    for (const Weight& w : kTable) {
        total += w.weight;
    }
    Act pick = Act::Sit;
    for (int attempt = 0; attempt < 3; ++attempt) {
        int r = RndI(0, total - 1);
        for (const Weight& w : kTable) {
            if (r < w.weight) {
                pick = w.act;
                break;
            }
            r -= w.weight;
        }
        if (pick != act_) {
            break;
        }
    }
    BeginAct(pick);
}

void Mate::BeginAct(Act a)
{
    act_ = a;
    actT_ = 0.0;
    actFlag_ = 0;
    counter_ = 0;
    timerA_ = 0.0;
    timerB_ = 0.0;
    bubble_ = -1;
    phase_ = 0;
    liftPos_ = 0.0;
    liftV_ = 0.0;
    const double s = scale_;
    switch (a) {
    case Act::Build: actDur_ = 8.4; break;
    case Act::Paint: {
        actDur_ = 9.0;
        LoadWallpaperThumb();
        std::fill(std::begin(revealed_), std::end(revealed_), false);
        const double w = surfRect_.right - surfRect_.left;
        easelSide_ = (w - relX_) >= relX_ ? 1 : -1;
        break;
    }
    case Act::Magic: actDur_ = 6.2; break;
    case Act::Cook: actDur_ = 7.6; break;
    case Act::Stare: {
        actDur_ = Rnd(2.5, 4.0);
        farT_ = 0.0;
        spinAcc_ = 0.0;
        spinIdle_ = 0.0;
        POINT c{};
        GetCursorPos(&c);
        lastAngle_ = std::atan2(c.y - (footY_ - 5.0 * s), c.x - footX_);
        break;
    }
    case Act::Dizzy: actDur_ = 3.0; break;
    case Act::Sit: actDur_ = Rnd(2.0, 4.0); break;
    case Act::Typing:
        actDur_ = Rnd(5.0, 12.0);
        counter_ = RndI(15, 35);
        break;
    case Act::Walk: {
        actDur_ = 8.0;
        const double w = surfRect_.right - surfRect_.left;
        const double m = 10.0 * s;
        targetRel_ = Rnd(m, std::max(m, w - m));
        if (std::abs(targetRel_ - relX_) < 15.0 * s) {
            targetRel_ = std::clamp(relX_ + (Rnd() < 0.5 ? -1.0 : 1.0) * Rnd(20.0 * s, 60.0 * s), m, std::max(m, w - m));
        }
        dir_ = targetRel_ >= relX_ ? 1 : -1;
        break;
    }
    case Act::Wave: actDur_ = 2.5; break;
    case Act::Look: actDur_ = 4.0; break;
    case Act::Dance: actDur_ = Rnd(4.0, 6.0); break;
    case Act::Sleep: actDur_ = Rnd(6.0, 12.0); break;
    case Act::Think: actDur_ = 2.6; break;
    case Act::Sneeze: actDur_ = 2.0; break;
    case Act::React: actDur_ = 1.2; break;
    }
}

void Mate::BeginReact(int kind)
{
    BeginAct(Act::React);
    reactKind_ = kind;
    switch (kind) {
    case 0:
        hopV_ = 45.0;
        Spawn('h', kCrabX + 1, kCrabY - 6, -4.0, -10.0, 1.2, kHeart);
        Spawn('h', kCrabX + 10, kCrabY - 8, 4.0, -12.0, 1.2, kHeart);
        actDur_ = 1.2;
        break;
    case 1:
        Spawn('h', kCrabX + 5, kCrabY - 7, 0.0, -10.0, 1.0, kHeart);
        actDur_ = 1.0;
        break;
    default:
        hopV_ = 30.0;
        Spawn('!', kCrabX + 6, kCrabY - 7, 0.0, -8.0, 0.9, kBang);
        actDur_ = 0.9;
        break;
    }
}

void Mate::StartRetreat(bool fast)
{
    fastRetreat_ = fast;
    bubble_ = -1;
    Enter(State::Retreat);
}

void Mate::StartFall(double vx, double vy)
{
    scanner_.Refresh();
    fallScan_ = 0.25;
    surface_ = Surface::None;
    surfHwnd_ = nullptr;
    side_ = 0;
    bubble_ = -1;
    vx_ = vx;
    vy_ = vy;
    SetZ(ZMode::Free);
    Enter(State::Falling);
}

void Mate::OnClick()
{
    if (state_ == State::Peek) {
        shy_ = true;
        stateT_ = std::max(stateT_, 2.3);
        return;
    }
    if (state_ != State::Active) {
        return;
    }
    if (act_ == Act::Sleep) {
        BeginReact(2);
        return;
    }
    const int k = RndI(0, 9);
    if (k < 4) {
        BeginReact(0);
    } else if (k < 7) {
        BeginReact(1);
    } else {
        StartRetreat(true);
    }
}

void Mate::SetZ(ZMode mode)
{
    if (mode != zmode_) {
        zmode_ = mode;
        zTopDone_ = false;
    }
}

void Mate::UpdateZ(bool force)
{
    constexpr UINT kFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER;
    switch (zmode_) {
    case ZMode::Behind:
        if (surfHwnd_ && (force || GetWindow(surfHwnd_, GW_HWNDNEXT) != crab_)) {
            SetWindowPos(crab_, surfHwnd_, 0, 0, 0, 0, kFlags);
        }
        break;
    case ZMode::Above:
        if (surfHwnd_) {
            HWND prev = GetWindow(surfHwnd_, GW_HWNDPREV);
            if (prev == crab_ && !force) {
                break;
            }
            HWND after = HWND_TOP;
            if (prev && prev != crab_ && !(GetWindowLongPtrW(prev, GWL_EXSTYLE) & WS_EX_TOPMOST)) {
                after = prev;
            }
            SetWindowPos(crab_, after, 0, 0, 0, 0, kFlags);
        }
        break;
    case ZMode::Free:
        if (force || !zTopDone_) {
            SetWindowPos(crab_, HWND_TOP, 0, 0, 0, 0, kFlags);
            zTopDone_ = true;
        }
        break;
    }
}

void Mate::Tick(double dt)
{
    clock_ += dt;
    blinkT_ -= dt;
    if (blinkT_ < -0.13) {
        blinkT_ = Rnd(2.0, 5.0);
    }

    for (Particle& p : particles_) {
        p.life -= dt;
        p.vy += p.ay * dt;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& p) { return p.life <= 0.0; }),
                     particles_.end());

    if (hopV_ != 0.0 || hop_ > 0.0) {
        hopV_ -= 160.0 * dt;
        hop_ += hopV_ * dt;
        if (hop_ <= 0.0) {
            hop_ = 0.0;
            hopV_ = 0.0;
        }
    }

    if (state_ != State::Hidden) {
        stateT_ += dt;
    }
    pose_ = Pose{};
    shake_ = 0;
    lift_ = 0;
    ladderH_ = 0;
    easel_ = false;
    chuteFold_ = -1;
    stars_ = false;
    StepState(dt);
    if (state_ == State::Hidden) {
        return;
    }
    if (pose_.eyes == Eyes::Open && blinkT_ < 0.0) {
        pose_.eyes = Eyes::Closed;
    }
    UpdateZ(false);
    Render();
}

void Mate::StepState(double dt)
{
    const double s = scale_;
    const double t = stateT_;

    switch (state_) {
    case State::Hidden:
        if (!settings_.paused) {
            hideNext_ -= dt;
            if (hideNext_ <= 0.0) {
                Appear(true);
            }
        }
        return;

    case State::SidePeek: {
        if (!FollowSurface()) {
            Hide(Rnd(10.0, 30.0));
            return;
        }
        SetZ(ZMode::Behind);
        const double ex = side_ < 0 ? surfRect_.left : surfRect_.right;
        const double hidden = ex - side_ * 9.0 * s;
        const double out = ex - side_ * 1.0 * s;
        double k = 1.0;
        if (t < 0.5) {
            k = Smooth(t / 0.5);
        } else if (t >= 1.9) {
            k = 1.0 - Smooth((t - 1.9) / 0.4);
        }
        footX_ = Lerp(hidden, out, k);
        footY_ = surfRect_.top + relY_;
        pose_.lookX = side_;
        if (t > 1.2 && t < 1.32) {
            pose_.eyes = Eyes::Closed;
        }
        if (t >= 0.6 && stateFlag_ == 0) {
            stateFlag_ = 1;
            Spawn('?', kCrabX + (side_ < 0 ? 1 : 11), kCrabY - 7, 0.0, -6.0, 1.0, kWhite);
        }
        if (t >= 2.3) {
            const HWND w = surfHwnd_;
            scanner_.Refresh();
            side_ = 0;
            particles_.clear();
            if (!PickTopSpot(w) && !PickTopSpot(nullptr)) {
                PickGroundSpot();
            }
            shy_ = false;
            Enter(State::Peek);
        }
        return;
    }

    case State::Peek: {
        if (!FollowSurface()) {
            Hide(Rnd(10.0, 30.0));
            return;
        }
        SetZ(surface_ == Surface::Window ? ZMode::Behind : ZMode::Free);
        footX_ = surfRect_.left + relX_;
        const double down = edge_ + 11.0 * s;
        const double peek = edge_ + 5.0 * s;
        if (t < 0.5) {
            footY_ = Lerp(down, peek, Smooth(t / 0.5));
        } else if (t < 2.3) {
            footY_ = peek;
            pose_.lookX = t < 1.1 ? -1 : (t < 1.7 ? 1 : 0);
            if (t > 1.95 && t < 2.07) {
                pose_.eyes = Eyes::Closed;
            }
        } else if (shy_) {
            if (stateFlag_ == 0) {
                stateFlag_ = 1;
                Spawn('!', kCrabX + 6, kCrabY - 10, 0.0, -6.0, 0.8, kBang);
            }
            pose_.eyes = Eyes::Surprised;
            footY_ = Lerp(peek, down, Smooth((t - 2.3) / 0.3));
            if (t >= 2.7) {
                Hide(Rnd(8.0, 25.0));
            }
        } else {
            footY_ = peek;
            Enter(State::Emerge);
        }
        return;
    }

    case State::Emerge: {
        if (!FollowSurface()) {
            Hide(Rnd(10.0, 30.0));
            return;
        }
        footX_ = surfRect_.left + relX_;
        if (t < 0.25) {
            SetZ(surface_ == Surface::Window ? ZMode::Behind : ZMode::Free);
            footY_ = Lerp(edge_ + 5.0 * s, edge_, Smooth(t / 0.25));
        } else {
            SetZ(surface_ == Surface::Window ? ZMode::Above : ZMode::Free);
            footY_ = edge_;
            if (stateFlag_ == 0) {
                stateFlag_ = 1;
                hopV_ = 40.0;
            }
            pose_.eyes = Eyes::Happy;
            if (t >= 0.8) {
                StartActivities();
            }
        }
        return;
    }

    case State::Active:
        if (!FollowSurface()) {
            StartFall(0.0, 0.0);
            return;
        }
        SetZ(surface_ == Surface::Window ? ZMode::Above : ZMode::Free);
        coverCheck_ -= dt;
        if (coverCheck_ <= 0.0) {
            coverCheck_ = 0.5;
            if (surface_ == Surface::Window) {
                scanner_.Refresh();
                bool covered = false;
                const auto& wins = scanner_.Windows();
                for (size_t i = 0; i < wins.size(); ++i) {
                    if (wins[i].hwnd == surfHwnd_) {
                        POINT head{static_cast<LONG>(footX_), static_cast<LONG>(footY_ - 5.0 * s)};
                        covered = scanner_.IsCovered(head, i);
                        break;
                    }
                }
                coveredT_ = covered ? coveredT_ + 0.5 : 0.0;
                if (coveredT_ >= 2.0) {
                    coveredT_ = 0.0;
                    Hide(Rnd(5.0, 15.0));
                    return;
                }
            } else if (static_cast<int>(clock_ / 0.5) % 4 == 0) {
                zTopDone_ = false;
            }
        }
        actT_ += dt;
        CheckStare(dt);
        StepActivity(dt);
        if (state_ == State::Active) {
            footX_ = surfRect_.left + relX_;
            footY_ = edge_;
        }
        return;

    case State::Retreat: {
        if (!FollowSurface()) {
            Hide();
            return;
        }
        footX_ = surfRect_.left + relX_;
        const double start = fastRetreat_ ? 0.5 : 1.2;
        const double slide = fastRetreat_ ? 0.25 : 0.5;
        if (t < start) {
            SetZ(surface_ == Surface::Window ? ZMode::Above : ZMode::Free);
            footY_ = edge_;
            if (fastRetreat_) {
                pose_.eyes = Eyes::Surprised;
                if (stateFlag_ == 0) {
                    stateFlag_ = 1;
                    hopV_ = 30.0;
                    Spawn('!', kCrabX + 6, kCrabY - 8, 0.0, -8.0, 0.8, kBang);
                }
            } else {
                pose_.eyes = Eyes::Happy;
                pose_.right = (static_cast<int>(t / 0.2) % 2) ? Claw::High : Claw::Up;
            }
        } else {
            SetZ(surface_ == Surface::Window ? ZMode::Behind : ZMode::Free);
            footY_ = Lerp(edge_, edge_ + 11.0 * s, Smooth((t - start) / slide));
            if (t >= start + slide) {
                Hide();
            }
        }
        return;
    }

    case State::Falling: {
        const double prevY = footY_;
        vy_ = std::min(vy_ + 600.0 * s * dt, 400.0 * s);
        footY_ += vy_ * dt;
        footX_ += vx_ * dt;
        vx_ *= std::exp(-1.5 * dt);
        const double vl = GetSystemMetrics(SM_XVIRTUALSCREEN);
        const double vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        footX_ = std::clamp(footX_, vl + 10.0 * s, vl + vw - 10.0 * s);
        pose_.legs = Legs::Spread;
        pose_.left = Claw::High;
        pose_.right = Claw::High;
        pose_.eyes = Eyes::Surprised;
        SetZ(ZMode::Free);
        fallScan_ -= dt;
        if (fallScan_ <= 0.0) {
            scanner_.Refresh();
            fallScan_ = 0.25;
        }
        if (vy_ > 0.0 && TryLand(prevY)) {
            POINT foot{static_cast<LONG>(footX_), static_cast<LONG>(footY_)};
            SetScale(ScaleAt(foot));
            if (vy_ > 250.0 * s) {
                Spawn('*', kCrabX - 2, kCrabY + 2, -10.0, -8.0, 0.6, kBang);
                Spawn('*', kCrabX + 15, kCrabY + 2, 10.0, -8.0, 0.6, kBang);
            }
            return;
        }
        const double vt = GetSystemMetrics(SM_YVIRTUALSCREEN);
        const double vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        if (footY_ > vt + vh + 100.0 * s) {
            Hide();
        }
        return;
    }

    case State::Dragged: {
        POINT c{};
        GetCursorPos(&c);
        if (dt > 0.0) {
            mouseVx_ = 0.6 * mouseVx_ + 0.4 * (c.x - lastMouse_.x) / dt;
            mouseVy_ = 0.6 * mouseVy_ + 0.4 * (c.y - lastMouse_.y) / dt;
        }
        lastMouse_ = c;
        footX_ = c.x - grabDx_;
        footY_ = c.y - grabDy_;
        SetZ(ZMode::Free);
        const int ph = static_cast<int>(t / 0.1) % 2;
        pose_.eyes = Eyes::Surprised;
        pose_.legs = ph ? Legs::Spread : Legs::Stand;
        pose_.left = ph ? Claw::High : Claw::Up;
        pose_.right = ph ? Claw::Up : Claw::High;
        timerA_ -= dt;
        if (timerA_ <= 0.0) {
            timerA_ = 0.7;
            Spawn('d', kCrabX + 14, kCrabY - 3, 2.0, 8.0, 0.6, kDrop);
        }
        return;
    }

    case State::Land:
        if (!FollowSurface()) {
            StartFall(0.0, 0.0);
            return;
        }
        SetZ(surface_ == Surface::Window ? ZMode::Above : ZMode::Free);
        footX_ = surfRect_.left + relX_;
        footY_ = edge_;
        pose_.body = Body::Squash;
        pose_.eyes = t < 0.12 ? Eyes::Closed : Eyes::Open;
        if (t > 0.3) {
            if (actsLeft_ < 2) {
                actsLeft_ = RndI(2, 4);
            }
            Enter(State::Active);
            act_ = Act::Sit;
            NextActivity();
        }
        return;

    case State::Parachute:
        if (stateFlag_ == 0) {
            footY_ += 45.0 * s * dt;
            footX_ = chuteX0_ + std::sin(t * 1.3) * 6.0 * s;
            chuteFold_ = 0;
            pose_.left = Claw::High;
            pose_.right = Claw::High;
            pose_.legs = (static_cast<int>(t / 0.5) % 2) ? Legs::LiftLeft : Legs::LiftRight;
            pose_.lookX = static_cast<int>(t / 1.4) % 3 - 1;
            pose_.lookY = (static_cast<int>(t / 2.1) % 2) ? 1 : 0;
            SetZ(ZMode::Free);
            if (footY_ >= surfRect_.bottom) {
                footY_ = surfRect_.bottom;
                relX_ = footX_ - surfRect_.left;
                edge_ = surfRect_.bottom;
                stateFlag_ = 1;
                landT_ = t;
            }
        } else {
            const double lt = t - landT_;
            footY_ = edge_;
            pose_.body = lt < 0.25 ? Body::Squash : Body::Normal;
            pose_.eyes = lt < 0.25 ? Eyes::Closed : Eyes::Happy;
            chuteFold_ = lt < 0.25 ? 1 : (lt < 0.6 ? 2 : -1);
            if (lt >= 0.6 && stateFlag_ == 1) {
                stateFlag_ = 2;
                Puff(kCrabX + 8, kCrabY + 8, 6, kWhite);
            }
            if (lt >= 1.0) {
                StartActivities();
            }
        }
        return;
    }
}

void Mate::CheckStare(double dt)
{
    stareCooldown_ -= dt;
    const double s = scale_;
    POINT c{};
    GetCursorPos(&c);
    const double dx = c.x - footX_;
    const double dy = c.y - (footY_ - 5.0 * s);
    const bool isNear = std::hypot(dx, dy) < 70.0 * s;
    const bool interruptible = act_ == Act::Sit || act_ == Act::Typing || act_ == Act::Walk || act_ == Act::Look;
    if (isNear && !cursorNear_ && interruptible && stareCooldown_ <= 0.0) {
        if (Rnd() < 0.3) {
            BeginAct(Act::Stare);
            stareCooldown_ = Rnd(15.0, 30.0);
        } else {
            stareCooldown_ = 3.0;
        }
    }
    cursorNear_ = isNear;
}

void Mate::LoadWallpaperThumb()
{
    wchar_t path[MAX_PATH] = {};
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, path, 0) && path[0] != L'\0') {
        Gdiplus::Bitmap src(path);
        if (src.GetLastStatus() == Gdiplus::Ok && src.GetWidth() > 0) {
            Gdiplus::Bitmap dst(7, 5, PixelFormat32bppARGB);
            Gdiplus::Graphics g(&dst);
            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
            g.DrawImage(&src, 0, 0, 7, 5);
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 7; ++x) {
                    Gdiplus::Color col;
                    dst.GetPixel(x, y, &col);
                    painting_[y * 7 + x] = (static_cast<uint32_t>(col.GetR()) << 16) |
                                           (static_cast<uint32_t>(col.GetG()) << 8) | col.GetB();
                }
            }
            return;
        }
    }
    const COLORREF d = GetSysColor(COLOR_DESKTOP);
    for (uint32_t& p : painting_) {
        const int v = RndI(-14, 14);
        const int r = std::clamp(static_cast<int>(GetRValue(d)) + v, 0, 255);
        const int g = std::clamp(static_cast<int>(GetGValue(d)) + v, 0, 255);
        const int b = std::clamp(static_cast<int>(GetBValue(d)) + v, 0, 255);
        p = (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }
}

void Mate::StepActivity(double dt)
{
    const double s = scale_;
    POINT cur{};
    GetCursorPos(&cur);
    const double dx = cur.x - footX_;
    const double dy = cur.y - (footY_ - 5.0 * s);
    const double dist = std::hypot(dx, dy);
    const bool breathe = std::fmod(clock_, 1.8) < 0.18;
    bool done = actT_ >= actDur_;

    switch (act_) {
    case Act::Sit:
        if (breathe) {
            pose_.body = Body::Crouch;
        }
        pose_.lookX = static_cast<int>(actT_ / 1.3) % 3 - 1;
        break;

    case Act::Typing:
        pose_.keyboard = true;
        if (timerB_ > 0.0) {
            timerB_ -= dt;
            pose_.lookY = -1;
            if (std::fmod(timerB_, 0.5) < 0.2) {
                pose_.body = Body::Crouch;
            }
            if (timerB_ <= 0.0) {
                counter_ = RndI(15, 35);
            }
        } else {
            timerA_ -= dt;
            if (timerA_ <= 0.0) {
                timerA_ = Rnd(0.07, 0.16);
                actFlag_ = RndI(0, 1);
                pressedKey_ = actFlag_ == 0 ? RndI(2, 7) : RndI(8, 13);
                if (Rnd() < 0.35) {
                    const char ch = kCodeGlyphs[RndI(0, static_cast<int>(sizeof(kCodeGlyphs)) - 2)];
                    Spawn(ch, kCrabX + pressedKey_ - 1, kCrabY - 6, Rnd(-4.0, 4.0), Rnd(-15.0, -9.0), Rnd(0.8, 1.3), kCode);
                }
                if (--counter_ <= 0) {
                    timerB_ = Rnd(0.8, 1.6);
                }
            }
            const bool down = timerA_ > 0.04;
            pose_.left = (actFlag_ == 0 && down) ? Claw::Down : Claw::Mid;
            pose_.right = (actFlag_ == 1 && down) ? Claw::Down : Claw::Mid;
            pose_.pressedKey = down ? pressedKey_ : -1;
            pose_.lookY = 1;
        }
        break;

    case Act::Walk: {
        relX_ += 16.0 * s * dt * dir_;
        const bool reached = (dir_ > 0 && relX_ >= targetRel_) || (dir_ < 0 && relX_ <= targetRel_);
        const int ph = static_cast<int>(actT_ / 0.11) % 4;
        pose_.legs = ph == 1 ? Legs::LiftLeft : (ph == 3 ? Legs::LiftRight : Legs::Stand);
        pose_.lookX = dir_;
        done = reached || actT_ >= actDur_;
        break;
    }

    case Act::Wave: {
        const int side = Sign(dx) == 0 ? 1 : Sign(dx);
        pose_.eyes = Eyes::Happy;
        pose_.lookX = side;
        const Claw c = (static_cast<int>(actT_ / 0.18) % 2) ? Claw::High : Claw::Up;
        if (side > 0) {
            pose_.right = c;
        } else {
            pose_.left = c;
        }
        if (actFlag_ == 0) {
            actFlag_ = 1;
            if (Rnd() < 0.5) {
                Spawn('h', kCrabX + (side > 0 ? 13 : -2), kCrabY - 6, 0.0, -9.0, 1.2, kHeart);
            }
        }
        break;
    }

    case Act::Look:
        pose_.lookX = std::abs(dx) > 12.0 * s ? Sign(dx) : 0;
        pose_.lookY = dy < -30.0 * s ? -1 : (dy > 30.0 * s ? 1 : 0);
        if (dist < 25.0 * s) {
            pose_.eyes = Eyes::Happy;
        }
        if (breathe) {
            pose_.body = Body::Crouch;
        }
        break;

    case Act::Dance: {
        const int beat = static_cast<int>(actT_ / 0.28);
        const bool even = beat % 2 == 0;
        pose_.body = even ? Body::Crouch : Body::Normal;
        shake_ = even ? -1 : 1;
        pose_.left = even ? Claw::High : Claw::Mid;
        pose_.right = even ? Claw::Mid : Claw::High;
        pose_.legs = even ? Legs::LiftLeft : Legs::LiftRight;
        pose_.eyes = (beat / 2) % 2 ? Eyes::Happy : Eyes::Open;
        timerA_ -= dt;
        if (timerA_ <= 0.0) {
            timerA_ = 0.55;
            Spawn('n', kCrabX + RndI(-3, 14), kCrabY - 3, Rnd(-6.0, 6.0), -12.0, 1.2, kNote);
        }
        break;
    }

    case Act::Sleep:
        pose_.eyes = Eyes::Closed;
        pose_.left = Claw::Down;
        pose_.right = Claw::Down;
        pose_.body = std::fmod(actT_, 2.0) < 1.0 ? Body::Crouch : Body::Normal;
        timerA_ -= dt;
        if (timerA_ <= 0.0) {
            timerA_ = 1.1;
            Spawn('Z', kCrabX + 12, kCrabY - 2, 5.0, -7.0, 2.2, kWhite);
        }
        if (actT_ > 1.5 && dist < 22.0 * s) {
            BeginReact(2);
            return;
        }
        break;

    case Act::Think:
        if (actT_ < 1.8) {
            bubble_ = std::min(3, 1 + static_cast<int>(actT_ / 0.5));
            pose_.lookX = 1;
            pose_.lookY = -1;
            pose_.right = Claw::Up;
        } else {
            bubble_ = -1;
            if (actFlag_ == 0) {
                actFlag_ = 1;
                Spawn('!', kCrabX + 6, kCrabY - 8, 0.0, -10.0, 0.9, kBang);
                hopV_ = 35.0;
            }
            pose_.eyes = Eyes::Happy;
        }
        break;

    case Act::Sneeze:
        if (actT_ < 0.9) {
            pose_.lookY = -1;
            pose_.eyes = actT_ > 0.5 ? Eyes::Closed : Eyes::Open;
            pose_.body = std::fmod(actT_, 0.3) < 0.15 ? Body::Normal : Body::Crouch;
        } else if (actT_ < 1.4) {
            if (actFlag_ == 0) {
                actFlag_ = 1;
                for (int i = 0; i < 3; ++i) {
                    Spawn('*', kCrabX + 6 + RndI(-6, 6), kCrabY + 1, Rnd(-20.0, 20.0), Rnd(-15.0, -5.0), 0.7, kWhite);
                }
            }
            pose_.eyes = Eyes::Closed;
            pose_.body = Body::Squash;
            shake_ = (static_cast<int>(actT_ / 0.05) % 2) ? 1 : -1;
        }
        break;

    case Act::Build: StepBuild(dt); break;
    case Act::Paint: StepPaint(dt); break;
    case Act::Magic: StepMagic(dt); break;
    case Act::Cook: StepCook(dt); break;

    case Act::Stare: {
        pose_.lookX = std::abs(dx) > 8.0 * s ? Sign(dx) : 0;
        pose_.lookY = dy < -12.0 * s ? -1 : (dy > 12.0 * s ? 1 : 0);
        if (std::abs(dx) > 40.0 * s) {
            shake_ = Sign(dx);
        }
        farT_ = dist > 90.0 * s ? farT_ + dt : 0.0;
        if (farT_ > 1.0) {
            done = true;
        }
        if (dist < 110.0 * s && dist > 6.0 * s) {
            const double angle = std::atan2(dy, dx);
            double d = angle - lastAngle_;
            while (d > kPi) {
                d -= 2.0 * kPi;
            }
            while (d < -kPi) {
                d += 2.0 * kPi;
            }
            lastAngle_ = angle;
            if (std::abs(d) > 0.02) {
                if (spinAcc_ != 0.0 && Sign(d) != Sign(spinAcc_)) {
                    spinAcc_ = 0.0;
                }
                spinAcc_ += d;
                spinIdle_ = 0.0;
                actDur_ = std::max(actDur_, actT_ + 1.0);
                done = false;
            } else {
                spinIdle_ += dt;
            }
            if (spinIdle_ > 1.5) {
                spinAcc_ = 0.0;
            }
            if (std::abs(spinAcc_) >= 4.0 * kPi) {
                BeginAct(Act::Dizzy);
                return;
            }
        } else {
            spinAcc_ = 0.0;
        }
        break;
    }

    case Act::Dizzy:
        stars_ = actT_ < 2.6;
        if (actT_ < 2.6) {
            pose_.eyes = Eyes::Dizzy;
            pose_.frame = static_cast<int>(actT_ / 0.07);
            shake_ = static_cast<int>(std::lround(std::sin(actT_ * 5.0)));
            pose_.legs = (static_cast<int>(actT_ / 0.25) % 2) ? Legs::LiftLeft : Legs::LiftRight;
            pose_.body = (static_cast<int>(actT_ / 0.5) % 2) ? Body::Crouch : Body::Normal;
        } else {
            pose_.eyes = Eyes::Closed;
            shake_ = (static_cast<int>(actT_ / 0.05) % 2) ? 1 : -1;
        }
        break;

    case Act::React:
        if (reactKind_ == 1) {
            shake_ = (static_cast<int>(actT_ / 0.08) % 2) ? 1 : -1;
            pose_.legs = shake_ > 0 ? Legs::LiftLeft : Legs::LiftRight;
            pose_.eyes = Eyes::Happy;
        } else if (reactKind_ == 0) {
            pose_.eyes = Eyes::Happy;
            pose_.left = Claw::High;
            pose_.right = Claw::High;
        } else {
            pose_.eyes = actT_ < 0.6 ? Eyes::Surprised : Eyes::Open;
        }
        break;
    }

    if (done) {
        bubble_ = -1;
        NextActivity();
    }
}

void Mate::StepBuild(double dt)
{
    const double t = actT_;
    pose_.hat = Hat::HardHat;
    if (t < 0.6) {
        pose_.hatDrop = static_cast<int>(std::lround((1.0 - t / 0.6) * 12.0));
        pose_.lookY = -1;
        return;
    }
    if (t < 0.75) {
        pose_.body = Body::Crouch;
    }
    if (phase_ == 0) {
        phase_ = 1;
        Puff(kCrabX + 8, kCrabY - 2, 5, kWhite);
    }
    if (t < 6.0) {
        ladderH_ = 12;
    }
    if (t < 0.9) {
        return;
    }
    if (t < 3.3) {
        const int step = std::min(6, static_cast<int>((t - 0.9) / 0.4) + 1);
        lift_ = 2 * step;
        pose_.legs = step % 2 ? Legs::LiftLeft : Legs::LiftRight;
        pose_.left = step % 2 ? Claw::High : Claw::Up;
        pose_.right = step % 2 ? Claw::Up : Claw::High;
        pose_.lookY = -1;
    } else if (t < 6.0) {
        lift_ = 12;
        pose_.tool = Tool::Hammer;
        pose_.toolSide = 1;
        const int k = static_cast<int>((t - 3.3) / 0.3);
        const bool up = k % 2 == 0;
        pose_.right = up ? Claw::High : Claw::Up;
        pose_.lookX = 1;
        pose_.lookY = -1;
        if (!up && counter_ != k) {
            counter_ = k;
            const double hx = kCrabX + 17;
            const double hy = kCrabY - 12 - 3;
            Spawn('*', hx, hy - 2, Rnd(-12.0, 12.0), Rnd(-14.0, -5.0), 0.4, kBang);
            if (Rnd() < 0.5) {
                Spawn('s', hx + 1, hy - 3, Rnd(-6.0, 10.0), Rnd(-12.0, -6.0), 0.5, kWhite);
            }
        }
    } else {
        if (phase_ < 2) {
            phase_ = 2;
            Puff(kCrabX + 8, kCrabY + 3, 6, kWhite);
            liftPos_ = 12.0;
            liftV_ = 0.0;
        }
        if (t < 6.35) {
            lift_ = 12;
        } else if (phase_ == 2) {
            liftV_ -= 200.0 * dt;
            liftPos_ += liftV_ * dt;
            if (liftPos_ <= 0.0) {
                liftPos_ = 0.0;
                phase_ = 3;
                landT_ = t;
                Spawn('*', kCrabX - 2, kCrabY + 6, -10.0, -8.0, 0.5, kBang);
                Spawn('*', kCrabX + 16, kCrabY + 6, 10.0, -8.0, 0.5, kBang);
            }
            lift_ = static_cast<int>(std::lround(liftPos_));
        }
        if (phase_ < 3) {
            pose_.eyes = Eyes::Surprised;
            pose_.legs = Legs::Spread;
            pose_.left = Claw::High;
            pose_.right = Claw::High;
        } else {
            const double lt = t - landT_;
            pose_.body = lt < 0.25 ? Body::Squash : Body::Normal;
            pose_.eyes = Eyes::Dizzy;
            pose_.frame = static_cast<int>(lt / 0.07);
            stars_ = true;
        }
    }
}

void Mate::StepPaint(double dt)
{
    (void)dt;
    const double t = actT_;
    pose_.hat = Hat::Beret;
    if (t < 0.5) {
        pose_.hatDrop = static_cast<int>(std::lround((1.0 - t / 0.5) * 10.0));
    }
    const double puffX = easelSide_ > 0 ? kCrabX + 22 : kCrabX - 7;
    if (t >= 0.4 && t < 8.7) {
        easel_ = true;
        if (phase_ == 0) {
            phase_ = 1;
            Puff(puffX, kCrabY + 4, 5, kWhite);
        }
    }
    if (t < 0.5) {
        return;
    }
    pose_.tool = Tool::Brush;
    pose_.toolSide = easelSide_;
    pose_.brushColor = brushColor_;
    Claw& arm = easelSide_ > 0 ? pose_.right : pose_.left;
    if (t < 1.5) {
        pose_.lookX = -easelSide_;
        pose_.lookY = -1;
    } else if (t < 7.5) {
        const double c = std::fmod(t - 1.5, 1.2);
        if (c < 0.4) {
            pose_.lookX = -easelSide_;
            pose_.lookY = -1;
        } else {
            pose_.lookX = easelSide_;
            const int k = static_cast<int>((c - 0.4) / 0.15);
            const bool dab = k % 2 == 0;
            arm = dab ? Claw::Mid : Claw::Up;
            const int dabId = static_cast<int>((t - 1.5) / 1.2) * 10 + k + 1;
            if (dab && counter_ != dabId) {
                counter_ = dabId;
                for (int n = 0; n < 2; ++n) {
                    int hidden[35];
                    int count = 0;
                    for (int i = 0; i < 35; ++i) {
                        if (!revealed_[i]) {
                            hidden[count++] = i;
                        }
                    }
                    if (count == 0) {
                        break;
                    }
                    const int idx = hidden[RndI(0, count - 1)];
                    revealed_[idx] = true;
                    brushColor_ = painting_[idx];
                }
            }
        }
    } else {
        std::fill(std::begin(revealed_), std::end(revealed_), true);
        pose_.eyes = Eyes::Happy;
        if (phase_ == 1) {
            phase_ = 2;
            hopV_ = 35.0;
            Spawn('h', kCrabX + 5, kCrabY - 8, 0.0, -9.0, 1.2, kHeart);
        }
        if (t >= 8.7 && phase_ == 2) {
            phase_ = 3;
            Puff(puffX, kCrabY + 4, 6, kWhite);
        }
    }
}

void Mate::StepMagic(double dt)
{
    (void)dt;
    const double t = actT_;
    pose_.hat = Hat::TopHat;
    if (t < 0.5) {
        pose_.hatDrop = static_cast<int>(std::lround((1.0 - t / 0.5) * 12.0));
        return;
    }
    pose_.tool = Tool::Wand;
    pose_.toolSide = 1;
    const double tipX = kCrabX + 16;
    if (t < 5.0) {
        const double c = std::fmod(t - 0.5, 0.9);
        const int k = static_cast<int>((t - 0.5) / 0.9);
        const bool flick = c < 0.25;
        pose_.right = flick ? Claw::High : Claw::Up;
        const double tipY = kCrabY + (flick ? 0 : 2) - 5;
        if (counter_ != k + 1) {
            counter_ = k + 1;
            dir_ = Rnd() < 0.5 ? -1 : 1;
            const double vx = dir_ * Rnd(16.0, 24.0);
            const double vy = -Rnd(10.0, 16.0);
            Spawn('s', tipX - 1, kCrabY - 6, vx, vy, 1.1, kBang);
            for (int j = 1; j <= 3; ++j) {
                Spawn('p', tipX - vx * 0.05 * j, kCrabY - 5 - vy * 0.05 * j, vx, vy, 1.1, kBang, 0.0, 0.85 - 0.2 * j);
            }
        }
        pose_.eyes = flick ? Eyes::Happy : Eyes::Open;
        pose_.lookX = dir_;
        pose_.lookY = -1;
        if (Rnd() < 0.12) {
            Spawn('p', tipX, tipY, Rnd(-3.0, 3.0), 6.0, 0.6, kWhite, 0.0, 0.8);
        }
    } else {
        if (phase_ == 0) {
            phase_ = 1;
            for (int i = 0; i < 8; ++i) {
                const double a = 2.0 * kPi * i / 8.0;
                Spawn('s', tipX - 1, kCrabY - 6, std::cos(a) * 18.0, std::sin(a) * 14.0, 0.7, kBang);
            }
        }
        if (t > 5.3) {
            pose_.body = Body::Crouch;
            pose_.eyes = Eyes::Happy;
            pose_.right = Claw::Mid;
        } else {
            pose_.right = Claw::High;
        }
    }
}

void Mate::StepCook(double dt)
{
    const double t = actT_;
    pose_.hat = Hat::Toque;
    if (t < 0.5) {
        pose_.hatDrop = static_cast<int>(std::lround((1.0 - t / 0.5) * 12.0));
        return;
    }
    pose_.tool = Tool::Pan;
    pose_.toolSide = 1;
    pose_.right = Claw::Mid;
    pose_.lookX = 1;
    if (t < 6.0) {
        const double c = std::fmod(t - 0.5, 1.3);
        const int k = static_cast<int>((t - 0.5) / 1.3);
        if (c < 0.15) {
            pose_.toolLift = 2;
        }
        if (counter_ != k + 1) {
            counter_ = k + 1;
            const int n = RndI(2, 3);
            for (int i = 0; i < n; ++i) {
                const int kind = RndI(0, 2);
                const char ch = kind == 0 ? 'c' : (kind == 1 ? 'p' : 't');
                const uint32_t col = kind == 0 ? kCarrot : (kind == 1 ? kPea : kTomato);
                const double vy = -Rnd(26.0, 34.0);
                Spawn(ch, kCrabX + 20 + RndI(0, 3), kCrabY + 3, Rnd(-3.0, 3.0), vy, 2.0 * -vy / 70.0, col, 70.0);
            }
        }
        pose_.lookY = (c > 0.15 && c < 0.9) ? -1 : 0;
        if (c >= 0.9 && c < 1.1) {
            pose_.eyes = Eyes::Happy;
        }
        timerA_ -= dt;
        if (timerA_ <= 0.0) {
            timerA_ = 0.6;
            Spawn('~', kCrabX + 21 + RndI(-1, 1), kCrabY + 1, Rnd(-2.0, 2.0), -8.0, 1.0, kSteam, 0.0, 0.6);
        }
    } else {
        pose_.eyes = Eyes::Happy;
        pose_.lookX = 0;
        if (t > 6.2 && t < 6.5) {
            pose_.body = Body::Crouch;
        }
        if (phase_ == 0) {
            phase_ = 1;
            Spawn('h', kCrabX + 6, kCrabY - 9, 0.0, -9.0, 1.2, kHeart);
        }
    }
}

void Mate::Render()
{
    if (!bits_) {
        return;
    }
    canvas_.Clear();
    const int ox = kCrabX + shake_;
    const int oy = kCrabY - static_cast<int>(std::lround(hop_)) - lift_;
    if (ladderH_ > 0) {
        canvas_.DrawLadder(kCrabX + 5, kCrabY + kCrabH - 1, ladderH_);
    }
    if (easel_) {
        canvas_.DrawEasel(easelSide_ > 0 ? kCrabX + 18 : kCrabX - 11, kCrabY - 1, painting_, revealed_);
    }
    if (chuteFold_ == 2) {
        canvas_.DrawParachute(ox + 8, oy - 14, 2);
    }
    canvas_.DrawCrab(pose_, ox, oy);
    if (chuteFold_ == 0 || chuteFold_ == 1) {
        canvas_.DrawParachute(ox + 8, oy - 14, chuteFold_);
    }
    if (stars_) {
        const int sy = pose_.hat != Hat::None ? oy - 6 : oy - 3;
        for (int k = 0; k < 3; ++k) {
            const double a = clock_ * 5.0 + k * 2.0 * kPi / 3.0;
            const int x = ox + 7 + static_cast<int>(std::lround(8.0 * std::cos(a)));
            const int y = sy + static_cast<int>(std::lround(2.0 * std::sin(a)));
            canvas_.DrawGlyph('s', x - 1, y - 1, kBang, 255);
        }
    }
    if (bubble_ >= 0) {
        canvas_.DrawBubble(ox + 9, oy - 11, bubble_);
    }
    for (const Particle& p : particles_) {
        const int alpha = static_cast<int>(255.0 * std::min(1.0, p.life / 0.35) * p.alpha);
        canvas_.DrawGlyph(p.ch, static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y)), p.color, alpha);
    }
    const int w = canvas_.PixelWidth();
    const int h = canvas_.PixelHeight();
    std::memcpy(bits_, canvas_.Data(), static_cast<size_t>(w) * h * 4);

    SIZE size{w, h};
    POINT src{0, 0};
    POINT dst{static_cast<LONG>(std::lround(footX_)) - kFootLX * scale_, static_cast<LONG>(std::lround(footY_)) - h};
    BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(crab_, nullptr, &dst, &size, memDc_, &src, 0, &bf, ULW_ALPHA);
}
