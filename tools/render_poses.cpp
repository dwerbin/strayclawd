#include <windows.h>

#include <cmath>
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
constexpr int kScale = 4;
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
        if (skipCrab && x >= kCrabX + 2 && x <= kCrabX + 26) {
            continue;
        }
        c.DrawGlyph('c', x, y, kRain, 200);
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
        sc.back = [](Canvas& c, int, int) { c.DrawPlant(kCrabX + 20, kFeetRow - 1, 4); };
        sc.front = [](Canvas& c, int, int) {
            c.DrawGlyph('c', kCrabX + 22, kCrabY + 0, kRain, 255);
            c.DrawGlyph('c', kCrabX + 21, kCrabY + 3, kRain, 200);
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
        sc.dy = -16;
        sc.front = [](Canvas& c, int ox, int oy) { c.DrawParachute(ox + 8, oy - 14, 0); };
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
    return s;
}

void FillRows(std::vector<uint32_t>& img, int y0, int y1, uint32_t rgb)
{
    const int w = kW * kScale;
    for (int y = y0 * kScale; y < y1 * kScale; ++y) {
        for (int x = 0; x < w; ++x) {
            img[static_cast<size_t>(y) * w + x] = 0xFF000000 | rgb;
        }
    }
}

void Background(std::vector<uint32_t>& img)
{
    FillRows(img, 0, kFeetRow, kDesk);
    FillRows(img, kFeetRow, kFeetRow + 1, kTitleEdge);
    FillRows(img, kFeetRow + 1, kFeetRow + 5, kTitleBar);
    FillRows(img, kFeetRow + 5, kH, kWindow);
}

void Over(std::vector<uint32_t>& img, const uint32_t* src, int fromRow, int toRow)
{
    const int w = kW * kScale;
    for (int y = fromRow * kScale; y < toRow * kScale; ++y) {
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

}

int wmain(int argc, wchar_t** argv)
{
    const std::wstring outDir = argc > 1 ? argv[1] : L".";
    Gdiplus::GdiplusStartupInput in;
    ULONG_PTR token = 0;
    Gdiplus::GdiplusStartup(&token, &in, nullptr);
    CLSID png{};
    if (!PngEncoder(&png)) {
        std::fwprintf(stderr, L"PNG encoder not found\n");
        return 1;
    }
    const int w = kW * kScale;
    const int h = kH * kScale;
    int failed = 0;
    for (const Scene& sc : Scenes()) {
        Canvas c;
        c.Resize(kW, kH, kScale);
        const int ox = kCrabX + sc.dx;
        const int oy = kCrabY + sc.dy;
        if (sc.back) {
            sc.back(c, ox, oy);
        }
        c.DrawCrab(sc.pose, ox, oy);
        if (sc.front) {
            sc.front(c, ox, oy);
        }
        std::vector<uint32_t> img(static_cast<size_t>(w) * h);
        Background(img);
        Over(img, c.Data(), 0, sc.peek ? kFeetRow : kH);

        Gdiplus::Bitmap bmp(w, (kBottom - kTop) * kScale, w * 4, PixelFormat32bppARGB,
                            reinterpret_cast<BYTE*>(img.data() + static_cast<size_t>(kTop) * kScale * w));
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
