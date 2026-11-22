#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include <objidl.h>
namespace Gdiplus {
using std::max;
using std::min;
}
#include <gdiplus.h>

#include "../src/Canvas.h"

namespace {

constexpr int kW = 56;
constexpr int kH = 60;
int gScale = 4;
constexpr int kTop = 8;
constexpr int kBottom = 56;
constexpr int kCrabX = 20;
constexpr int kCrabY = 38;
constexpr int kFeetRow = kCrabY + kCrabH;
constexpr double kPi = 3.14159265358979323846;

constexpr uint32_t kDesk = 0x5A6B82;
constexpr uint32_t kTitleBar = 0x3B414B;
constexpr uint32_t kTitleEdge = 0x59616E;
constexpr uint32_t kWindow = 0x2E333B;

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
constexpr uint32_t kRain = 0x7FB2FF;
constexpr uint32_t kBobber = 0xE8505B;
constexpr uint32_t kBalls[3] = {0xE8505B, 0xF2C14E, 0x3E7CD8};
constexpr uint32_t kConfetti[4] = {0xE86FB0, 0x4FC3E8, 0xF2C14E, 0x5DBB4A};
constexpr uint32_t kSpellRed = 0xFF5A5A;
constexpr uint32_t kFishColor = 0x5FA8D3;
constexpr uint32_t kBootColor = 0x6B4A2B;

/** One picture: what is drawn before, the crab pose, what is drawn after. */
struct Scene {
    const char* name;
    Pose pose;
    int dx = 0;
    int dy = 0;
    std::function<void(Canvas&, int, int)> back;
    std::function<void(Canvas&, int, int)> front;
    bool peek = false;
    int edgeX = kW;
    int padX = 0;
    int padTop = 0;
    bool noCrab = false;
};

void Stars(Canvas& c, int ox, int oy, bool hat)
{
    const int sy = hat ? oy - 6 : oy - 3;
    for (int k = 0; k < 3; ++k) {
        const double a = 1.5 + k * 2.0 * kPi / 3.0;
        const int x = ox + 7 + static_cast<int>(std::lround(8.0 * std::cos(a)));
        const int y = sy + static_cast<int>(std::lround(2.0 * std::sin(a)));
        c.DrawGlyph('s', x - 1, y - 1, kBang, 255);
    }
}

void Rain(Canvas& c, bool skipCrab)
{
    unsigned seed = 7;
    for (int i = 0; i < 26; ++i) {
        seed = seed * 1103515245u + 12345u;
        const int x = 2 + static_cast<int>((seed >> 8) % (kW - 4));
        seed = seed * 1103515245u + 12345u;
        const int y = static_cast<int>((seed >> 8) % (kFeetRow - 6));
        if (skipCrab && std::abs(x - kCrabX - 12) <= 12 && y > kCrabY - 9) {
            continue;
        }
        c.DrawGlyph('c', x, y, kRain, 200);
    }
}

void Mat(Canvas& c)
{
    const int x0 = kCrabX - 3;
    for (int x = x0; x < x0 + 22; ++x) {
        c.Put(x, kFeetRow - 2, (x - x0) % 5 == 0 ? 0xB79BE0 : 0x8E6BBF);
        c.Put(x, kFeetRow - 1, 0x6A4C96);
    }
}

std::vector<Scene> Scenes()
{
    std::vector<Scene> s;
    auto add = [&s](const char* name) -> Scene& {
        s.push_back(Scene{});
        s.back().name = name;
        return s.back();
    };

    {
        Scene& sc = add("peek");
        sc.pose.lookX = -1;
        sc.dy = 5;
        sc.peek = true;
    }
    {
        Scene& sc = add("typing");
        sc.pose.keyboard = true;
        sc.pose.left = Claw::Down;
        sc.pose.pressedKey = 4;
        sc.pose.lookY = 1;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('{', kCrabX + 2, kCrabY - 12, kCode, 255);
            c.DrawGlyph(';', kCrabX + 9, kCrabY - 17, kCode, 200);
            c.DrawGlyph('}', kCrabX + 13, kCrabY - 9, kCode, 255);
        };
    }
    {
        Scene& sc = add("wave");
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.pose.right = Claw::High;
        sc.front = [](Canvas& c, int, int) { c.DrawGlyph('h', kCrabX + 13, kCrabY - 10, kHeart, 255); };
    }
    {
        Scene& sc = add("dance");
        sc.pose.body = Body::Crouch;
        sc.pose.left = Claw::High;
        sc.pose.legs = Legs::LiftLeft;
        sc.pose.eyes = Eyes::Happy;
        sc.dx = -1;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('n', kCrabX - 2, kCrabY - 9, kNote, 255);
            c.DrawGlyph('n', kCrabX + 13, kCrabY - 15, kNote, 200);
        };
    }
    {
        Scene& sc = add("sleep");
        sc.pose.hat = Hat::Nightcap;
        sc.pose.eyes = Eyes::Closed;
        sc.pose.left = Claw::Down;
        sc.pose.right = Claw::Down;
        sc.pose.body = Body::Crouch;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('Z', kCrabX + 14, kCrabY - 8, kWhite, 255);
            c.DrawGlyph('Z', kCrabX + 19, kCrabY - 16, kWhite, 170);
        };
    }
    {
        Scene& sc = add("think");
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.pose.right = Claw::Up;
        sc.front = [](Canvas& c, int ox, int oy) { c.DrawBubble(ox + 9, oy - 11, 3); };
    }
    {
        Scene& sc = add("diy");
        sc.pose.hat = Hat::HardHat;
        sc.pose.tool = Tool::Hammer;
        sc.pose.right = Claw::High;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.dy = -12;
        sc.back = [](Canvas& c, int, int) { c.DrawLadder(kCrabX + 5, kCrabY + kCrabH - 1, 12); };
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('*', kCrabX + 18, kCrabY - 20, kBang, 255);
            c.DrawGlyph('s', kCrabX + 22, kCrabY - 24, kWhite, 220);
        };
    }
    {
        Scene& sc = add("painter");
        sc.pose.hat = Hat::Beret;
        sc.pose.tool = Tool::Brush;
        sc.pose.brushColor = 0x4F8FD8;
        sc.pose.lookX = 1;
        sc.back = [](Canvas& c, int, int) {
            static uint32_t painting[35];
            static bool revealed[35];
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 7; ++x) {
                    const int i = y * 7 + x;
                    const bool hill = y >= 3 || (y == 2 && x >= 2 && x <= 4);
                    painting[i] = hill ? (y == 4 ? 0x3F8F3A : 0x5DBB4A) : (y == 0 ? 0x4F8FD8 : 0x7FB2FF);
                    if (x == 5 && y == 0) {
                        painting[i] = 0xF2C14E;
                    }
                    revealed[i] = (i * 7) % 10 < 7;
                }
            }
            c.DrawEasel(kCrabX + 18, kCrabY - 1, painting, revealed);
        };
    }
    {
        Scene& sc = add("magician");
        sc.pose.hat = Hat::TopHat;
        sc.pose.tool = Tool::Wand;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.front = [](Canvas& c, int, int) {
            const int stars[][3] = {{19, -10, 255}, {24, -16, 230}, {15, -19, 200}, {27, -8, 230}, {21, -24, 150}};
            for (const auto& st : stars) {
                c.DrawGlyph('s', kCrabX + st[0], kCrabY + st[1], kBang, st[2]);
            }
        };
    }
    {
        Scene& sc = add("young_wizard");
        sc.pose.roundGlasses = true;
        sc.pose.scar = true;
        sc.pose.scarf = true;
        sc.pose.tool = Tool::Wand;
        sc.pose.woodWand = true;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('s', kCrabX + 29, kCrabY - 2, kSpellRed, 255);
            for (int j = 1; j <= 3; ++j) {
                c.Put(kCrabX + 29 - j, kCrabY - 1, kSpellRed, 230 - 60 * j);
            }
        };
    }
    {
        Scene& sc = add("broom");
        sc.pose.roundGlasses = true;
        sc.pose.scar = true;
        sc.pose.scarf = true;
        sc.pose.broom = true;
        sc.pose.legs = Legs::Spread;
        sc.pose.eyes = Eyes::Happy;
        sc.dy = -10;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('p', kCrabX + 23, kCrabY - 3, kBang, 230);
            c.DrawGlyph('p', kCrabX + 26, kCrabY - 1, kBang, 170);
            c.DrawGlyph('p', kCrabX + 29, kCrabY - 3, kBang, 110);
        };
    }
    {
        Scene& sc = add("grey_wizard");
        sc.pose.hat = Hat::Wizard;
        sc.pose.beard = true;
        sc.pose.tool = Tool::Staff;
        sc.pose.right = Claw::High;
        sc.pose.glow = 180;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.front = [](Canvas& c, int, int) {
            for (int i = 0; i < 10; ++i) {
                const double a = 2.0 * kPi * i / 10.0;
                const int x = kCrabX + 15 + static_cast<int>(std::lround(std::cos(a) * 8.0));
                const int y = 17 + static_cast<int>(std::lround(std::sin(a) * 6.0));
                c.DrawGlyph('s', x - 1, y - 1, i % 2 ? kConfetti[1] : kConfetti[0], 255);
            }
        };
    }
    {
        Scene& sc = add("cook");
        sc.pose.hat = Hat::Toque;
        sc.pose.tool = Tool::Pan;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('c', kCrabX + 20, kCrabY - 9, kCarrot, 255);
            c.DrawGlyph('p', kCrabX + 24, kCrabY - 5, kPea, 255);
            c.DrawGlyph('t', kCrabX + 17, kCrabY - 3, kTomato, 255);
            c.DrawGlyph('~', kCrabX + 21, kCrabY - 14, kSteam, 150);
        };
    }
    {
        Scene& sc = add("fishing");
        sc.pose.tool = Tool::Rod;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::Surprised;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.front = [](Canvas& c, int ox, int oy) {
            c.DrawLine(ox + 20, oy - 4, ox + 24, oy - 12, 0xDDDDDD, 200);
            c.DrawGlyph('f', kCrabX + 20, kCrabY - 14, kFishColor, 255);
            c.DrawGlyph('!', kCrabX + 6, kCrabY - 10, kBang, 255);
        };
    }
    {
        Scene& sc = add("old_boot");
        sc.pose.tool = Tool::Rod;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::Closed;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.front = [](Canvas& c, int ox, int oy) {
            c.DrawLine(ox + 20, oy - 4, ox + 24, oy - 12, 0xDDDDDD, 200);
            c.DrawGlyph('b', kCrabX + 21, kCrabY - 14, kBootColor, 255);
            c.DrawGlyph('?', kCrabX + 6, kCrabY - 10, kWhite, 255);
        };
    }
    {
        Scene& sc = add("reading");
        sc.pose.glasses = true;
        sc.pose.book = 0;
        sc.pose.lookY = 1;
    }
    {
        Scene& sc = add("juggling");
        sc.pose.left = Claw::Up;
        sc.pose.lookY = -1;
        sc.pose.lookX = 1;
        sc.front = [](Canvas& c, int ox, int oy) {
            for (int k = 0; k < 3; ++k) {
                const double ph = 2.0 * std::fmod(0.35 + k / 3.0, 1.0);
                double x = 0.0;
                double y = 0.0;
                if (ph < 1.0) {
                    x = ox + 0.5 + 14.0 * ph;
                    y = oy - 1.0 - 12.0 * std::sin(kPi * ph);
                } else {
                    x = ox + 14.5 - 14.0 * (ph - 1.0);
                    y = oy - 1.0 - 4.0 * std::sin(kPi * (ph - 1.0));
                }
                c.DrawGlyph('t', static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y)), kBalls[k], 255);
            }
        };
    }
    {
        Scene& sc = add("watering");
        sc.pose.tool = Tool::Can;
        sc.pose.right = Claw::Up;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.back = [](Canvas& c, int, int) { c.DrawPlant(kCrabX + 20, kFeetRow - 1, 5); };
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('c', kCrabX + 22, kCrabY + 0, kRain, 255);
            c.DrawGlyph('c', kCrabX + 21, kCrabY + 3, kRain, 200);
        };
    }
    {
        Scene& sc = add("flower_boom");
        sc.pose.tool = Tool::Can;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::Mid;
        sc.pose.eyes = Eyes::Surprised;
        sc.pose.lookX = 1;
        sc.back = [](Canvas& c, int, int) { c.DrawPlant(kCrabX + 20, kFeetRow - 1, 0); };
        sc.front = [](Canvas& c, int, int) {
            const int cx = kCrabX + 22;
            const int cy = kFeetRow - 15;
            for (int i = 0; i < 14; ++i) {
                const double a = 2.0 * kPi * i / 14.0;
                const int x = cx + static_cast<int>(std::lround(std::cos(a) * 9.0));
                const int y = cy + static_cast<int>(std::lround(std::sin(a) * 7.0));
                c.DrawGlyph('p', x, y, i % 3 == 0 ? kNote : 0xE86FB0, 255);
            }
            c.DrawGlyph('*', cx - 1, cy - 2, kBang, 255);
            c.DrawGlyph('t', cx - 3, cy + 3, 0xBDBDBD, 200);
            c.DrawGlyph('t', cx + 2, cy + 1, 0xBDBDBD, 200);
        };
    }
    {
        Scene& sc = add("dusting");
        sc.pose.tool = Tool::Duster;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.front = [](Canvas& c, int, int) {
            const int puffs[][3] = {{39, 30, 0}, {42, 34, 1}, {45, 29, 0}, {41, 26, 1}, {47, 33, 0}};
            for (const auto& q : puffs) {
                c.DrawGlyph(q[2] ? 't' : 'p', q[0], q[1], 0xCFC6B0, 220);
            }
        };
    }
    {
        Scene& sc = add("sweeping");
        sc.pose.tool = Tool::Broom;
        sc.pose.toolLift = 1;
        sc.pose.legs = Legs::LiftLeft;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.front = [](Canvas& c, int ox, int) {
            const int cx = ox + 25;
            for (int j = 0; j < 3; ++j) {
                const int half = 3 - j;
                for (int x = cx - half; x <= cx + half; ++x) {
                    c.Put(x, kFeetRow - 1 - j, (x + j) % 3 == 0 ? 0xA89F8A : 0xCFC6B0);
                }
            }
            c.DrawGlyph('p', cx + 3, kFeetRow - 6, 0xCFC6B0, 220);
            c.DrawGlyph('p', cx + 6, kFeetRow - 4, 0xCFC6B0, 160);
        };
    }
    {
        Scene& sc = add("yoga_tree");
        sc.pose.hat = Hat::Headband;
        sc.pose.legs = Legs::LiftLeft;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::Closed;
        sc.dy = -2;
        sc.back = [](Canvas& c, int, int) { Mat(c); };
    }
    {
        Scene& sc = add("yoga_lotus");
        sc.pose.hat = Hat::Headband;
        sc.pose.legs = Legs::Spread;
        sc.pose.body = Body::Crouch;
        sc.pose.left = Claw::Mid;
        sc.pose.right = Claw::Mid;
        sc.pose.eyes = Eyes::Closed;
        sc.dy = -7;
        sc.back = [](Canvas& c, int, int) { Mat(c); };
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('s', kCrabX - 3, kCrabY - 6, kWhite, 200);
            c.DrawGlyph('s', kCrabX + 17, kCrabY - 9, kWhite, 170);
            c.DrawGlyph('s', kCrabX + 8, kCrabY - 14, kWhite, 130);
        };
    }
    {
        Scene& sc = add("fitness");
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.pose.barbell = -1;
        sc.pose.eyes = Eyes::Closed;
        sc.front = [](Canvas& c, int, int) { c.DrawGlyph('d', kCrabX + 16, kCrabY - 1, kDrop, 255); };
    }
    {
        Scene& sc = add("skateboard");
        sc.pose.skate = true;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.dy = -8;
    }
    {
        Scene& sc = add("off_the_edge");
        sc.pose.eyes = Eyes::Surprised;
        sc.dy = -2;
        sc.edgeX = kCrabX - 4;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('!', kCrabX + 6, kCrabY - 11, kBang, 255);
            c.DrawGlyph('k', kCrabX + 14, kFeetRow + 5, 0xB07A3B, 255);
        };
    }
    {
        Scene& sc = add("umbrella");
        sc.pose.tool = Tool::Umbrella;
        sc.pose.right = Claw::Up;
        sc.pose.lookY = -1;
        sc.back = [](Canvas& c, int, int) { Rain(c, true); };
    }
    {
        Scene& sc = add("rainbow");
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookY = -1;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.dy = -3;
        sc.back = [](Canvas& c, int, int) { c.DrawRainbow(kCrabX + 8, kCrabY + 6, 220); };
    }
    {
        Scene& sc = add("coffee");
        sc.pose.tool = Tool::Mug;
        sc.pose.right = Claw::Up;
        sc.pose.eyes = Eyes::Closed;
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('~', kCrabX + 17, kCrabY - 6, kSteam, 170);
            c.DrawGlyph('~', kCrabX + 18, kCrabY - 12, kSteam, 110);
        };
    }
    {
        Scene& sc = add("parachute");
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.pose.legs = Legs::LiftLeft;
        sc.pose.lookX = -1;
        sc.dy = -9;
        sc.front = [](Canvas& c, int ox, int oy) { c.DrawParachute(ox + 8, oy - 14, 0); };
    }
    {
        Scene& sc = add("rocket_build");
        sc.pose.tool = Tool::Hammer;
        sc.pose.right = Claw::Up;
        sc.pose.lookX = 1;
        sc.back = [](Canvas& c, int, int) { c.DrawRocket(kCrabX + 19, kFeetRow - 20, 3, 0); };
        sc.front = [](Canvas& c, int, int) { c.DrawGlyph('*', kCrabX + 17, kCrabY - 8, kBang, 255); };
    }
    {
        Scene& sc = add("rocket_launch");
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.pose.eyes = Eyes::None;
        sc.pose.lookY = -1;
        sc.dy = -14;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawRocket(ox + 1, oy - 10, 4, 1); };
        sc.front = [](Canvas& c, int, int) {
            const int puffs[][3] = {{24, 46, 0}, {30, 48, 1}, {20, 50, 1}, {33, 51, 0}, {27, 53, 1}, {16, 47, 0}};
            for (const auto& q : puffs) {
                c.DrawGlyph(q[2] ? 't' : 'p', q[0], q[1], 0xBDBDBD, 220);
            }
        };
    }
    {
        Scene& sc = add("dizzy");
        sc.pose.eyes = Eyes::Dizzy;
        sc.pose.frame = 3;
        sc.front = [](Canvas& c, int ox, int oy) { Stars(c, ox, oy, false); };
    }
    {
        Scene& sc = add("santa");
        sc.pose.hat = Hat::Santa;
        sc.pose.beard = true;
        sc.pose.eyes = Eyes::Happy;
        sc.back = [](Canvas& c, int, int) {
            const int flakes[][2] = {{5, 6}, {14, 18}, {44, 9}, {50, 24}, {9, 30}, {36, 2}, {28, 14}, {47, 38}};
            for (const auto& f : flakes) {
                c.DrawGlyph('p', f[0], f[1], kWhite, 230);
            }
        };
    }
    {
        Scene& sc = add("witch");
        sc.pose.hat = Hat::Witch;
        sc.pose.lookX = 1;
    }
    {
        Scene& sc = add("party");
        sc.pose.hat = Hat::Party;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int, int) {
            const int bits[][2] = {{4, 8}, {12, 20}, {46, 6}, {51, 22}, {8, 33}, {38, 3}, {25, 12}, {45, 35}, {17, 5}};
            int k = 0;
            for (const auto& b : bits) {
                c.DrawGlyph('p', b[0], b[1], kConfetti[k++ % 4], 255);
            }
        };
    }
    {
        Scene& sc = add("portal");
        sc.dx = -8;
        sc.pose.tool = Tool::PortalGun;
        sc.pose.saberColor = 0xFF9A1F;
        sc.pose.toolSide = 1;
        sc.pose.right = Claw::Mid;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.front = [](Canvas& c, int ox, int) {
            c.DrawPortal(ox + 28, kFeetRow, 1.0, 0, 3);
            c.DrawGlyph('s', ox + 25, kCrabY + 3, 0xFF9A1F, 230);
            c.DrawPortal(ox + 34, kTop + 6, 0.8, 1, 5);
            c.DrawGlyph('p', ox + 26, kTop + 12, 0x36C0F1, 200);
            c.DrawGlyph('p', ox + 22, kTop + 20, 0x36C0F1, 140);
        };
    }
    {
        Scene& sc = add("climb");
        sc.dx = -6;
        sc.dy = -10;
        sc.pose.eyes = Eyes::Closed;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.pose.right = Claw::High;
        sc.pose.left = Claw::Up;
        sc.pose.legs = Legs::LiftLeft;
        sc.back = [](Canvas& c, int ox, int) {
            const int x0 = ox + 16;
            const int top = kFeetRow - 12;
            for (int y = top; y < kH; ++y) {
                for (int x = x0; x < kW; ++x) {
                    const uint32_t col = y == top ? kTitleEdge : (y < top + 5 ? kTitleBar : 0x5A4A70);
                    c.Put(x, y, col);
                }
            }
        };
        sc.front = [](Canvas& c, int ox, int oy) {
            c.DrawGlyph('d', ox - 1, oy - 3, kDrop, 255);
            c.DrawGlyph('p', ox + 14, kFeetRow - 3, 0xCFC6B0, 200);
        };
    }
    {
        Scene& sc = add("duo_talk");
        sc.dx = -13;
        sc.pose.lookX = 1;
        sc.pose.mouth = true;
        sc.pose.right = Claw::Up;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            c.DrawCrab(other, ox + 26, oy);
            c.DrawSpeech(ox + 8, oy - 1, "...", 3);
        };
    }
    {
        Scene& sc = add("duo_toast");
        sc.dx = -14;
        sc.pose.lookX = 1;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.tool = Tool::Mug;
        sc.pose.right = Claw::High;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.eyes = Eyes::Happy;
            other.tool = Tool::Mug;
            other.toolSide = -1;
            other.left = Claw::High;
            c.DrawCrab(other, ox + 28, oy);
            c.DrawGlyph('*', ox + 21, oy - 5, kBang, 255);
            c.DrawGlyph('p', ox + 18, oy - 4, kWhite, 230);
            c.DrawGlyph('p', ox + 26, oy - 5, kWhite, 230);
        };
    }
    {
        Scene& sc = add("duo_cook");
        sc.dx = -17;
        sc.pose.hat = Hat::Toque;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.pose.right = Claw::Up;
        sc.back = [](Canvas& c, int ox, int) { c.DrawPot(ox + 25, kFeetRow - 1, 5); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.hat = Hat::Toque;
            other.lookX = -1;
            other.lookY = 1;
            other.left = Claw::High;
            c.DrawCrab(other, ox + 34, oy);
            c.DrawGlyph('c', ox + 30, oy - 3, kCarrot, 255);
            c.DrawGlyph('~', ox + 24, oy - 2, kSteam, 160);
        };
    }
    {
        Scene& sc = add("duo_rocket");
        sc.padX = 6;
        sc.padTop = 17;
        sc.dx = -17;
        sc.pose.hat = Hat::HardHat;
        sc.pose.lookX = 1;
        sc.pose.tool = Tool::Hammer;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawBigRocket(ox + 27, oy + kCrabH - 1, 5, 0); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.hat = Hat::HardHat;
            other.lookX = -1;
            other.lookY = -1;
            other.tool = Tool::Hammer;
            other.toolSide = -1;
            other.left = Claw::Up;
            c.DrawCrab(other, ox + 38, oy);
            c.DrawGlyph('*', ox + 18, oy - 6, kBang, 255);
            c.DrawSign(ox - 5, oy + kCrabH - 1);
        };
    }
    {
        Scene& sc = add("duo_bomb");
        sc.padX = 14;
        sc.dx = -28;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.pose.eyes = Eyes::Surprised;
        sc.pose.tool = Tool::Wrench;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawBomb(ox + 32, oy + kCrabH - 1, 4, 6); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.eyes = Eyes::Happy;
            other.tool = Tool::Mug;
            other.toolSide = 1;
            c.DrawCrab(other, ox + 54, oy);
            c.DrawGlyph('d', ox + 1, oy - 3, kDrop, 255);
            c.DrawSpeech(ox + 62, oy - 17, "THIS IS FINE.", 99);
        };
    }
    {
        Scene& sc = add("duo_saber");
        sc.dx = -14;
        sc.pose.lookX = 1;
        sc.pose.tool = Tool::Saber;
        sc.pose.saberColor = 0x4FC3E8;
        sc.pose.saberLen = 8;
        sc.pose.saberAngle = 1;
        sc.pose.right = Claw::Up;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.tool = Tool::Saber;
            other.toolSide = -1;
            other.saberColor = 0xFF4A4A;
            other.saberLen = 8;
            other.saberAngle = 1;
            other.left = Claw::Up;
            c.DrawCrab(other, ox + 28, oy);
            c.DrawGlyph('*', ox + 21, oy - 10, kBang, 255);
        };
    }
    {
        Scene& sc = add("duo_boost");
        sc.pose.eyes = Eyes::Closed;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.eyes = Eyes::Happy;
            other.lookY = -1;
            other.left = Claw::High;
            other.right = Claw::Up;
            c.DrawCrab(other, ox, oy - 10);
            c.DrawGlyph('d', ox + 16, oy + 1, kDrop, 255);
            c.DrawGlyph('!', ox + 7, oy - 18, kBang, 255);
        };
    }
    {
        Scene& sc = add("duo_pray");
        sc.dx = -17;
        sc.pose.robe = true;
        sc.pose.eyes = Eyes::Closed;
        sc.pose.lookX = 1;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int ox, int) { c.DrawAltar(ox + 25, kFeetRow - 1, 10, 220); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.robe = true;
            other.eyes = Eyes::Closed;
            other.lookX = -1;
            other.left = Claw::High;
            other.right = Claw::High;
            c.DrawCrab(other, ox + 34, oy);
            c.DrawGlyph('n', ox + 28, oy - 18, 0xB98CFF, 230);
        };
    }
    {
        Scene& sc = add("duo_boom");
        sc.padX = 14;
        sc.padTop = 14;
        sc.dx = -28;
        sc.noCrab = true;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawExplosion(ox + 35, oy + kCrabH - 1, 0.6); };
        sc.front = [](Canvas& c, int ox, int oy) {
            c.DrawGlyph('*', ox + 4, oy - 10, kBang, 220);
            c.DrawGlyph('p', ox + 64, oy - 14, kWhite, 200);
            c.DrawGlyph('*', ox + 70, oy - 2, kBang, 180);
            c.DrawGlyph('p', ox + 1, oy + 2, 0x39FF14, 200);
        };
    }
    {
        Scene& sc = add("duo_highfive");
        sc.dx = -8;
        sc.dy = -12;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.pose.legs = Legs::Spread;
        sc.pose.right = Claw::High;
        sc.pose.left = Claw::Mid;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.eyes = Eyes::Happy;
            other.lookX = -1;
            other.lookY = -1;
            other.legs = Legs::Spread;
            other.left = Claw::High;
            other.right = Claw::Mid;
            c.DrawCrab(other, ox + 16, oy);
            c.DrawGlyph('*', ox + 15, oy - 6, kBang, 255);
            c.DrawGlyph('s', ox + 8, oy - 4, kBang, 230);
            c.DrawGlyph('s', ox + 22, oy - 4, kBang, 230);
            c.DrawGlyph('p', ox + 11, oy - 8, kWhite, 220);
            c.DrawGlyph('p', ox + 20, oy - 8, kWhite, 220);
            c.DrawGlyph('*', ox + 4, kFeetRow - 3, kWhite, 160);
            c.DrawGlyph('*', ox + 26, kFeetRow - 3, kWhite, 160);
        };
    }
    {
        Scene& sc = add("duo_tug");
        sc.padX = 10;
        sc.dx = -24;
        sc.pose.lookX = 1;
        sc.pose.legs = Legs::Spread;
        sc.pose.eyes = Eyes::Closed;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawRope(ox + 16, ox + 40, oy + 4, 1); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.legs = Legs::Spread;
            c.DrawCrab(other, ox + 40, oy);
            c.DrawGlyph('d', ox + 1, oy - 3, kDrop, 255);
        };
    }
    {
        Scene& sc = add("duo_pong");
        sc.padX = 10;
        sc.dx = -24;
        sc.pose.lookX = 1;
        sc.pose.tool = Tool::Paddle;
        sc.pose.right = Claw::Up;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawPongTable(ox + 30, oy + kCrabH - 1); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.tool = Tool::Paddle;
            other.toolSide = -1;
            other.left = Claw::Up;
            c.DrawCrab(other, ox + 44, oy);
            c.DrawGlyph('t', ox + 37, oy - 2, kWhite, 255);
            c.DrawGlyph('p', ox + 34, oy, kWhite, 150);
            c.DrawGlyph('p', ox + 31, oy - 5, kWhite, 110);
            c.DrawGlyph('p', ox + 27, oy - 8, kWhite, 80);
            c.DrawGlyph('p', ox + 22, oy - 7, kWhite, 50);
        };
    }
    {
        Scene& sc = add("duo_rps");
        sc.dx = -12;
        sc.pose.lookX = 1;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.right = Claw::High;
        sc.pose.left = Claw::High;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.lookY = 1;
            other.eyes = Eyes::Closed;
            other.body = Body::Crouch;
            other.left = Claw::Down;
            other.right = Claw::Down;
            c.DrawCrab(other, ox + 24, oy);
            c.DrawSignBubble(ox + 8, oy - 1, 1);
            c.DrawSignBubble(ox + 32, oy - 1, 0);
        };
    }
    {
        Scene& sc = add("duo_selfie");
        sc.dx = -6;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.lookY = -1;
        sc.pose.tool = Tool::Phone;
        sc.pose.toolSide = -1;
        sc.pose.left = Claw::High;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.eyes = Eyes::Happy;
            other.lookY = -1;
            other.left = Claw::High;
            other.right = Claw::High;
            c.DrawCrab(other, ox + 18, oy);
            c.DrawGlyph('*', ox - 4, oy - 9, kWhite, 255);
            c.DrawGlyph('p', ox - 6, oy - 11, kWhite, 220);
            c.DrawGlyph('p', ox, oy - 11, kWhite, 220);
        };
    }
    {
        Scene& sc = add("duo_pair");
        sc.dx = -10;
        sc.pose.keyboard = true;
        sc.pose.left = Claw::Down;
        sc.pose.pressedKey = 4;
        sc.pose.lookY = 1;
        sc.pose.eyes = Eyes::Surprised;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.lookY = 1;
            other.left = Claw::High;
            c.DrawCrab(other, ox + 22, oy);
            c.DrawGlyph('g', ox + 6, oy - 9, 0xE23B3B, 255);
            c.DrawGlyph('!', ox + 28, oy - 9, 0xE23B3B, 255);
            c.DrawGlyph('{', ox + 1, oy - 15, kCode, 200);
            c.DrawGlyph(';', ox + 12, oy - 13, kCode, 200);
        };
    }
    {
        Scene& sc = add("duo_pillow");
        sc.dx = -10;
        sc.pose.lookX = 1;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.tool = Tool::Pillow;
        sc.pose.right = Claw::High;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.eyes = Eyes::Closed;
            other.tool = Tool::Pillow;
            other.toolSide = -1;
            other.left = Claw::Mid;
            c.DrawCrab(other, ox + 23, oy);
            c.DrawGlyph('p', ox + 18, oy - 8, kWhite, 255);
            c.DrawGlyph('p', ox + 22, oy - 11, 0xCFC6B0, 255);
            c.DrawGlyph('p', ox + 14, oy - 12, kWhite, 255);
            c.DrawGlyph('p', ox + 25, oy - 6, kWhite, 255);
        };
    }
    {
        Scene& sc = add("duo_leapfrog");
        sc.dx = -10;
        sc.dy = -12;
        sc.pose.lookX = 1;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.legs = Legs::Spread;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.body = Body::Crouch;
            other.eyes = Eyes::Closed;
            c.DrawCrab(other, ox + 12, oy + 12);
        };
    }
    {
        Scene& sc = add("duo_mirror");
        sc.dx = -12;
        sc.pose.lookX = 1;
        sc.pose.right = Claw::High;
        sc.pose.left = Claw::Down;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.left = Claw::High;
            other.right = Claw::Down;
            c.DrawCrab(other, ox + 26, oy);
            for (int y = oy - 4; y <= oy + 9; y += 2) {
                c.Put(ox + 20, y, 0xBFE4FF, 120);
            }
        };
    }
    {
        Scene& sc = add("duo_picnic");
        sc.padX = 12;
        sc.dx = -26;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.pose.eyes = Eyes::Surprised;
        sc.pose.tool = Tool::Sandwich;
        sc.pose.toolLift = 1;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawPicnic(ox + 30, oy + kCrabH - 1, ox + 38, true, -999); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.lookY = 1;
            other.eyes = Eyes::Surprised;
            other.tool = Tool::Sandwich;
            other.toolSide = -1;
            other.toolLift = 2;
            c.DrawCrab(other, ox + 44, oy);
            c.DrawGlyph('!', ox + 35, oy - 6, kBang, 255);
        };
    }
    {
        Scene& sc = add("duo_chess");
        sc.dx = -14;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawChess(ox + 23, oy + kCrabH - 1, 1, false); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.lookX = -1;
            other.lookY = 1;
            other.eyes = Eyes::Closed;
            c.DrawCrab(other, ox + 30, oy);
            c.DrawSpeech(ox + 38, oy - 1, "...", 3);
        };
    }
    {
        Scene& sc = add("duo_seesaw");
        sc.padX = 10;
        sc.padTop = 6;
        sc.dx = -24;
        sc.noCrab = true;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawSeesaw(ox + 31, oy + kCrabH - 1, 26, -5); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose a;
            a.lookX = 1;
            a.eyes = Eyes::Happy;
            a.left = Claw::High;
            a.right = Claw::High;
            c.DrawCrab(a, ox + 3, oy - 11);
            Pose b;
            b.lookX = -1;
            b.eyes = Eyes::Happy;
            c.DrawCrab(b, ox + 45, oy - 2);
        };
    }
    {
        Scene& sc = add("duo_deploy");
        sc.padX = 4;
        sc.dx = -18;
        sc.pose.lookX = 1;
        sc.pose.eyes = Eyes::Surprised;
        sc.pose.left = Claw::High;
        sc.back = [](Canvas& c, int ox, int oy) {
            c.DrawDeploy(ox + 25, oy + kCrabH - 1, true, 2);
            c.DrawGlyph('t', ox + 24, oy - 6, 0xBDBDBD, 200);
            c.DrawGlyph('p', ox + 27, oy - 9, 0xBDBDBD, 200);
        };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose other;
            other.eyes = Eyes::Closed;
            other.tool = Tool::Mug;
            other.toolSide = 1;
            c.DrawCrab(other, ox + 34, oy);
            c.DrawGlyph('!', ox + 6, oy - 9, 0xE23B3B, 255);
        };
    }
    {
        Scene& sc = add("trio_pyramid");
        sc.dx = -8;
        sc.noCrab = true;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose b;
            b.eyes = Eyes::Closed;
            b.left = Claw::High;
            b.right = Claw::High;
            b.lookX = 1;
            c.DrawCrab(b, ox, oy);
            b.lookX = -1;
            c.DrawCrab(b, ox + 16, oy);
            Pose t;
            t.eyes = Eyes::Happy;
            t.left = Claw::High;
            t.right = Claw::Up;
            c.DrawCrab(t, ox + 8, oy - 10);
            c.DrawGlyph('h', ox + 13, oy - 20, kHeart, 255);
        };
    }
    {
        Scene& sc = add("trio_band");
        sc.padX = 6;
        sc.dx = -20;
        sc.pose.tool = Tool::Guitar;
        sc.pose.toolLift = 1;
        sc.pose.right = Claw::Down;
        sc.pose.left = Claw::Up;
        sc.pose.eyes = Eyes::Happy;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose d;
            d.eyes = Eyes::Closed;
            d.left = Claw::High;
            c.DrawCrab(d, ox + 22, oy);
            c.DrawDrums(ox + 30, oy + kCrabH - 1, 1);
            Pose m;
            m.eyes = Eyes::Closed;
            m.lookY = -1;
            m.mouth = true;
            m.tool = Tool::Mic;
            m.toolSide = -1;
            m.left = Claw::Up;
            c.DrawCrab(m, ox + 44, oy);
            c.DrawGlyph('n', ox + 4, oy - 12, kNote, 255);
            c.DrawGlyph('n', ox + 50, oy - 14, kNote, 255);
            c.DrawGlyph('n', ox + 40, oy - 9, kNote, 200);
        };
    }
    {
        Scene& sc = add("trio_standup");
        sc.padX = 6;
        sc.dx = -20;
        sc.pose.eyes = Eyes::Closed;
        sc.pose.body = Body::Crouch;
        sc.back = [](Canvas& c, int ox, int oy) { c.DrawBoard(ox + 30, oy + kCrabH - 1, 4); };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose m;
            m.mouth = true;
            m.right = Claw::Up;
            c.DrawCrab(m, ox + 22, oy);
            Pose r;
            r.lookX = -1;
            c.DrawCrab(r, ox + 44, oy);
            c.DrawSpeech(ox + 30, oy - 1, "...", 3);
            c.DrawGlyph('Z', ox + 10, oy - 9, kWhite, 230);
        };
    }
    {
        Scene& sc = add("trio_ritual");
        sc.padX = 10;
        sc.padTop = 12;
        sc.dx = -24;
        sc.pose.robe = true;
        sc.pose.eyes = Eyes::Surprised;
        sc.pose.lookX = 1;
        sc.pose.lookY = -1;
        sc.pose.left = Claw::High;
        sc.pose.right = Claw::High;
        sc.back = [](Canvas& c, int ox, int oy) {
            c.DrawAltar(ox + 34, oy + kCrabH - 1, 10, 230);
            c.DrawDemon(ox + 34, oy + kCrabH - 41, 255);
        };
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose p;
            p.robe = true;
            p.eyes = Eyes::Surprised;
            p.lookY = -1;
            p.left = Claw::High;
            p.right = Claw::High;
            c.DrawCrab(p, ox + 26, oy - 5);
            p.lookX = -1;
            c.DrawCrab(p, ox + 52, oy);
            c.DrawGlyph('?', ox + 40, oy - 34, 0xFF4A4A, 255);
        };
    }
    {
        Scene& sc = add("trio_review");
        sc.padX = 4;
        sc.dx = -18;
        sc.pose.lookX = 1;
        sc.pose.lookY = 1;
        sc.pose.eyes = Eyes::Happy;
        sc.pose.right = Claw::Up;
        sc.front = [](Canvas& c, int ox, int oy) {
            Pose m;
            m.keyboard = true;
            m.lookY = 1;
            m.eyes = Eyes::Surprised;
            c.DrawCrab(m, ox + 20, oy);
            Pose r;
            r.lookX = -1;
            r.lookY = 1;
            r.eyes = Eyes::Sly;
            r.left = Claw::High;
            c.DrawCrab(r, ox + 40, oy);
            c.DrawSpeech(ox + 8, oy - 1, "LGTM", 9);
            c.DrawGlyph('!', ox + 46, oy - 9, 0xE23B3B, 255);
            c.DrawGlyph('d', ox + 34, oy - 2, kDrop, 255);
        };
    }
    {
        Scene& sc = add("trio_conga");
        sc.dx = -14;
        sc.noCrab = true;
        sc.front = [](Canvas& c, int ox, int oy) {
            for (int i = 0; i < 3; ++i) {
                Pose p;
                p.lookX = 1;
                p.eyes = Eyes::Happy;
                p.legs = i % 2 ? Legs::LiftLeft : Legs::LiftRight;
                p.right = Claw::Up;
                p.left = i == 1 ? Claw::High : Claw::Mid;
                c.DrawCrab(p, ox + i * 15 + (i % 2 ? 1 : 0), oy);
            }
            c.DrawGlyph('n', ox + 8, oy - 12, kNote, 255);
            c.DrawGlyph('n', ox + 34, oy - 13, kNote, 220);
        };
    }
    return s;
}

void FillRows(std::vector<uint32_t>& img, int y0, int y1, uint32_t rgb, int x1 = kW, int width = kW)
{
    const int w = width * gScale;
    for (int y = y0 * gScale; y < y1 * gScale; ++y) {
        for (int x = 0; x < x1 * gScale; ++x) {
            img[static_cast<size_t>(y) * w + x] = 0xFF000000 | rgb;
        }
    }
}

void Background(std::vector<uint32_t>& img, int edgeX, int width = kW, int height = kH, int feet = kFeetRow)
{
    FillRows(img, 0, height, kDesk, width, width);
    FillRows(img, feet, feet + 1, kTitleEdge, edgeX, width);
    FillRows(img, feet + 1, feet + 5, kTitleBar, edgeX, width);
    FillRows(img, feet + 5, height, kWindow, edgeX, width);
}

void Over(std::vector<uint32_t>& img, const uint32_t* src, int fromRow, int toRow, int width = kW)
{
    const int w = width * gScale;
    for (int y = fromRow * gScale; y < toRow * gScale; ++y) {
        for (int x = 0; x < w; ++x) {
            const size_t i = static_cast<size_t>(y) * w + x;
            const uint32_t s = src[i];
            const uint32_t a = s >> 24;
            const uint32_t d = img[i];
            const uint32_t inv = 255 - a;
            const uint32_t r = ((s >> 16) & 0xFF) + ((d >> 16) & 0xFF) * inv / 255;
            const uint32_t g = ((s >> 8) & 0xFF) + ((d >> 8) & 0xFF) * inv / 255;
            const uint32_t b = (s & 0xFF) + (d & 0xFF) * inv / 255;
            img[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
}

bool PngEncoder(CLSID* clsid)
{
    UINT num = 0;
    UINT size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) {
        return false;
    }
    std::vector<BYTE> buf(size);
    auto* codecs = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buf.data());
    Gdiplus::GetImageEncoders(num, size, codecs);
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(codecs[i].MimeType, L"image/png") == 0) {
            *clsid = codecs[i].Clsid;
            return true;
        }
    }
    return false;
}

double Hash(int i)
{
    unsigned x = static_cast<unsigned>(i) * 2654435761u;
    x ^= x >> 13;
    x *= 0x5bd1e995u;
    x ^= x >> 15;
    return (x & 0xFFFF) / 65536.0;
}

double Ease(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

void Particle(Canvas& c, char g, double x0, double y0, double vx, double vy, double ay, double life, double dt,
              uint32_t col, double a = 1.0)
{
    if (dt < 0.0 || dt >= life) {
        return;
    }
    const double x = x0 + vx * dt;
    const double y = y0 + vy * dt + 0.5 * ay * dt * dt;
    const int alpha = static_cast<int>(255.0 * std::min(1.0, (life - dt) / 0.35) * a);
    c.DrawGlyph(g, static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y)), col, alpha);
}

void SpinStars(Canvas& c, int ox, int oy, double t)
{
    for (int k = 0; k < 3; ++k) {
        const double a = t * 5.0 + k * 2.0 * kPi / 3.0;
        const int x = ox + 7 + static_cast<int>(std::lround(8.0 * std::cos(a)));
        const int y = oy - 3 + static_cast<int>(std::lround(2.0 * std::sin(a)));
        c.DrawGlyph('s', x - 1, y - 1, kBang, 255);
    }
}

/** One short animated clip: duration and a frame function of the time. */
struct Clip {
    double duration;
    std::function<void(double t, Canvas& c, int& edgeX, bool& clipBelow)> frame;
};

std::vector<Clip> Clips()
{
    std::vector<Clip> clips;

    clips.push_back({2.6, [](double t, Canvas& c, int&, bool& clipBelow) {
        clipBelow = true;
        Pose p;
        int oy = kCrabY + 10;
        if (t < 0.6) {
            oy = kCrabY + 10 - static_cast<int>(std::lround(5.0 * Ease(t / 0.6)));
        } else if (t < 1.4) {
            oy = kCrabY + 5;
            p.lookX = t < 1.0 ? -1 : 1;
        } else if (t < 1.7) {
            const double s = (t - 1.4) / 0.3;
            oy = kCrabY + static_cast<int>(std::lround(5.0 * (1.0 - s) - 4.0 * std::sin(kPi * s)));
        } else {
            oy = kCrabY;
            p.eyes = Eyes::Happy;
            p.lookX = 1;
            p.right = static_cast<int>(t / 0.18) % 2 ? Claw::High : Claw::Up;
        }
        c.DrawCrab(p, kCrabX, oy);
        Particle(c, 'h', kCrabX + 13, kCrabY - 6, 0.0, -9.0, 0.0, 1.0, t - 1.8, kHeart);
    }});

    clips.push_back({2.2, [](double t, Canvas& c, int&, bool&) {
        Pose p;
        p.keyboard = true;
        p.lookY = 1;
        const int k = static_cast<int>(t / 0.12);
        const bool down = std::fmod(t, 0.12) < 0.08;
        p.left = (k % 2 == 0 && down) ? Claw::Down : Claw::Mid;
        p.right = (k % 2 == 1 && down) ? Claw::Down : Claw::Mid;
        p.pressedKey = down ? (k % 2 == 0 ? 2 + (k * 3) % 6 : 8 + (k * 5) % 6) : -1;
        c.DrawCrab(p, kCrabX, kCrabY);
        const char glyphs[] = "{};<>/=01";
        for (int i = 0; i < 10; ++i) {
            Particle(c, glyphs[i % 9], kCrabX + 1 + (i * 5) % 13, kCrabY - 6, (Hash(i) - 0.5) * 8.0, -12.0, 0.0, 1.1,
                     t - 0.2 * i, kCode);
        }
    }});

    clips.push_back({2.2, [](double t, Canvas& c, int&, bool&) {
        Pose p;
        const int beat = static_cast<int>(t / 0.28);
        const bool even = beat % 2 == 0;
        p.body = even ? Body::Crouch : Body::Normal;
        p.left = even ? Claw::High : Claw::Mid;
        p.right = even ? Claw::Mid : Claw::High;
        p.legs = even ? Legs::LiftLeft : Legs::LiftRight;
        p.eyes = (beat / 2) % 2 ? Eyes::Happy : Eyes::Open;
        c.DrawCrab(p, kCrabX + (even ? -1 : 1), kCrabY);
        const int xs[4] = {-3, 14, 4, 10};
        for (int i = 0; i < 5; ++i) {
            Particle(c, 'n', kCrabX + xs[i % 4], kCrabY - 3, (Hash(i + 20) - 0.5) * 10.0, -12.0, 0.0, 1.2,
                     t - 0.45 * i, kNote);
        }
    }});

    clips.push_back({2.6, [](double t, Canvas& c, int&, bool&) {
        auto lift = [](double tt) { return 12.0 * std::sin(kPi * std::clamp(tt / 2.6, 0.0, 1.0)); };
        Pose p;
        p.roundGlasses = true;
        p.scar = true;
        p.scarf = true;
        p.broom = true;
        p.legs = Legs::Spread;
        p.eyes = Eyes::Happy;
        const int oy = kCrabY - static_cast<int>(std::lround(lift(t)));
        c.DrawCrab(p, kCrabX, oy);
        for (int i = 0; i < 16; ++i) {
            const double ti = 0.15 * i;
            Particle(c, 'p', kCrabX + 21, kCrabY + 8 - lift(ti), 7.0, (Hash(i + 40) - 0.5) * 4.0, 0.0, 0.7, t - ti,
                     kBang, 0.9);
        }
    }});

    clips.push_back({3.4, [](double t, Canvas& c, int&, bool&) {
        const int plantX = kCrabX + 22;
        const bool boom = t >= 1.6;
        c.DrawPlant(kCrabX + 20, kFeetRow - 1, boom ? 0 : 5);
        Pose p;
        if (t < 1.4) {
            p.tool = Tool::Can;
            p.right = Claw::Up;
            p.lookX = 1;
            p.lookY = 1;
        } else if (t < 1.6) {
            p.tool = Tool::Can;
            p.eyes = Eyes::Happy;
        } else if (t < 2.3) {
            p.tool = Tool::Can;
            p.eyes = Eyes::Surprised;
            p.left = Claw::High;
            p.right = Claw::High;
        } else {
            p.eyes = Eyes::Dizzy;
            p.frame = static_cast<int>(t / 0.07);
        }
        const int ox = kCrabX + ((t >= 1.6 && t < 2.3) ? (static_cast<int>(t / 0.05) % 2 ? 1 : -1) : 0);
        c.DrawCrab(p, ox, kCrabY);
        for (int i = 0; i < 15; ++i) {
            Particle(c, 'c', plantX - (i % 2), kCrabY + 1, 0.0, 10.0, 60.0, 0.25, t - 0.2 - 0.08 * i, 0x7FB2FF);
        }
        if (boom) {
            const double dt = t - 1.6;
            const double fy = kFeetRow - 15;
            for (int i = 0; i < 14; ++i) {
                const double a = 2.0 * kPi * i / 14.0;
                const double v = 18.0 + 12.0 * Hash(i + 60);
                Particle(c, 'p', plantX, fy, std::cos(a) * v, std::sin(a) * v * 0.8 - 8.0, 45.0, 1.1, dt,
                         i % 3 == 0 ? kNote : 0xE86FB0);
            }
            for (int i = 0; i < 3; ++i) {
                Particle(c, '*', plantX - 1 + (i - 1) * 3, fy - 1, (i - 1) * 6.0, -7.0, 0.0, 0.5, dt, kBang);
            }
            for (int i = 0; i < 4; ++i) {
                Particle(c, 't', plantX - 2 + i, fy + 2 - i, (i - 1.5) * 2.0, -5.0, 0.0, 1.4, dt, 0xBDBDBD, 0.8);
            }
        }
        if (t >= 2.3) {
            SpinStars(c, ox, kCrabY, t);
        }
    }});

    clips.push_back({3.8, [](double t, Canvas& c, int& edgeX, bool&) {
        edgeX = 40;
        const double stopT = 1.6;
        const double ox = 2.0 + 22.0 * std::min(t, stopT);
        Pose p;
        double oy = kCrabY - 2;
        if (t < stopT) {
            p.skate = true;
            p.left = Claw::Up;
            p.right = Claw::Up;
            p.eyes = Eyes::Happy;
            p.lookX = 1;
        } else if (t < 2.1) {
            p.lookX = 1;
        } else if (t < 2.6) {
            p.lookY = 1;
        } else if (t < 3.1) {
            p.eyes = Eyes::Surprised;
        } else {
            const double ft = t - 3.1;
            const bool ph = static_cast<int>(ft / 0.08) % 2 == 0;
            p.eyes = Eyes::Surprised;
            p.left = ph ? Claw::High : Claw::Up;
            p.right = ph ? Claw::Up : Claw::High;
            p.legs = ph ? Legs::LiftLeft : Legs::LiftRight;
            oy += 0.5 * 200.0 * ft * ft;
        }
        const int shake = (t >= 2.6 && t < 3.1) ? (static_cast<int>(t / 0.05) % 2 ? 1 : -1) : 0;
        c.DrawCrab(p, static_cast<int>(std::lround(ox)) + shake, static_cast<int>(std::lround(oy)));
        Particle(c, 'k', ox + 2.0, kFeetRow, 45.0, 0.0, 150.0, 1.5, t - stopT, 0xB07A3B);
        Particle(c, '!', ox + 6.0, kCrabY - 11, 0.0, -8.0, 0.0, 0.8, t - 2.6, kBang);
    }});

    clips.push_back({3.0, [](double t, Canvas& c, int&, bool&) {
        const double rise = t > 1.3 ? 0.5 * 90.0 * (t - 1.3) * (t - 1.3) : 0.0;
        const int oy = kCrabY - 2 - static_cast<int>(std::lround(rise));
        const int shake = (t >= 0.9 && t < 1.3) ? (static_cast<int>(t / 0.04) % 2 ? 1 : -1) : 0;
        const int flame = t >= 0.9 ? 1 + static_cast<int>(t / 0.06) % 2 : 0;
        c.DrawRocket(kCrabX + 1 + shake, oy - 10, 4, flame);
        Pose p;
        p.left = Claw::High;
        p.right = Claw::High;
        p.lookY = -1;
        p.eyes = Eyes::None;
        c.DrawCrab(p, kCrabX + shake, oy);
        for (int n = 0; n < 3; ++n) {
            Particle(c, static_cast<char>('3' - n), kCrabX + 6, kCrabY - 18, 0.0, -4.0, 0.0, 0.3, t - 0.3 * n, kWhite);
        }
        for (int i = 0; i < 30; ++i) {
            const double ti = 0.9 + 0.06 * i;
            const double r = ti > 1.3 ? 0.5 * 90.0 * (ti - 1.3) * (ti - 1.3) : 0.0;
            Particle(c, i % 2 ? 'p' : 't', kCrabX + 5 + 6.0 * Hash(i + 80), kFeetRow + 1 - r, (Hash(i + 90) - 0.5) * 16.0,
                     -3.0, 0.0, 1.2, t - ti, 0xBDBDBD, 0.85);
        }
    }});

    return clips;
}

int WriteClips(const std::wstring& path)
{
    const int w = kW * gScale;
    const int h = kH * gScale;
    const int outH = (kBottom - kTop) * gScale;
    std::vector<std::vector<uint8_t>> frames;
    for (const Clip& clip : Clips()) {
        const int n = static_cast<int>(std::lround(clip.duration / 0.1));
        for (int f = 0; f < n; ++f) {
            Canvas c;
            c.Resize(kW, kH, gScale);
            int edgeX = kW;
            bool clipBelow = false;
            clip.frame(f * 0.1, c, edgeX, clipBelow);
            std::vector<uint32_t> img(static_cast<size_t>(w) * h);
            Background(img, edgeX);
            Over(img, c.Data(), 0, clipBelow ? kFeetRow : kH);
            std::vector<uint8_t> rgb(static_cast<size_t>(w) * outH * 3);
            for (int y = 0; y < outH; ++y) {
                for (int x = 0; x < w; ++x) {
                    const uint32_t v = img[static_cast<size_t>(y + kTop * gScale) * w + x];
                    uint8_t* d = &rgb[(static_cast<size_t>(y) * w + x) * 3];
                    d[0] = static_cast<uint8_t>(v >> 16);
                    d[1] = static_cast<uint8_t>(v >> 8);
                    d[2] = static_cast<uint8_t>(v);
                }
            }
            frames.push_back(std::move(rgb));
        }
    }
    FILE* out = nullptr;
    if (_wfopen_s(&out, path.c_str(), L"wb") != 0 || !out) {
        std::fwprintf(stderr, L"Cannot write %ls\n", path.c_str());
        return 1;
    }
    const int32_t header[3] = {w, outH, static_cast<int32_t>(frames.size())};
    std::fwrite(header, sizeof(header), 1, out);
    for (const auto& f : frames) {
        std::fwrite(f.data(), 1, f.size(), out);
    }
    std::fclose(out);
    std::wprintf(L"%ls: %d frames of %dx%d\n", path.c_str(), static_cast<int>(frames.size()), w, outH);
    return 0;
}

}

int wmain(int argc, wchar_t** argv)
{
    if (argc > 2 && std::wstring(argv[1]) == L"--clips") {
        gScale = 5;
        return WriteClips(argv[2]);
    }
    const std::wstring outDir = argc > 1 ? argv[1] : L".";
    Gdiplus::GdiplusStartupInput in;
    ULONG_PTR token = 0;
    Gdiplus::GdiplusStartup(&token, &in, nullptr);
    CLSID png{};
    if (!PngEncoder(&png)) {
        std::fwprintf(stderr, L"PNG encoder not found\n");
        return 1;
    }
    int failed = 0;
    for (const Scene& sc : Scenes()) {
        const int lw = kW + 2 * sc.padX;
        const int lh = kH + sc.padTop;
        const int w = lw * gScale;
        const int h = lh * gScale;
        const int feet = kFeetRow + sc.padTop;
        Canvas c;
        c.Resize(lw, lh, gScale);
        const int ox = kCrabX + sc.padX + sc.dx;
        const int oy = kCrabY + sc.padTop + sc.dy;
        if (sc.back) {
            sc.back(c, ox, oy);
        }
        if (!sc.noCrab) {
            c.DrawCrab(sc.pose, ox, oy);
        }
        if (sc.front) {
            sc.front(c, ox, oy);
        }
        std::vector<uint32_t> img(static_cast<size_t>(w) * h);
        Background(img, sc.edgeX == kW ? lw : sc.edgeX, lw, lh, feet);
        Over(img, c.Data(), 0, sc.peek ? feet : lh, lw);

        Gdiplus::Bitmap bmp(w, (kBottom - kTop + sc.padTop) * gScale, w * 4, PixelFormat32bppARGB,
                            reinterpret_cast<BYTE*>(img.data() + static_cast<size_t>(kTop) * gScale * w));
        const std::string name(sc.name);
        const std::wstring path = outDir + L"\\" + std::wstring(name.begin(), name.end()) + L".png";
        if (bmp.Save(path.c_str(), &png, nullptr) != Gdiplus::Ok) {
            std::fwprintf(stderr, L"Cannot write %ls\n", path.c_str());
            ++failed;
        } else {
            std::wprintf(L"%ls\n", path.c_str());
        }
    }
    Gdiplus::GdiplusShutdown(token);
    return failed ? 1 : 0;
}
