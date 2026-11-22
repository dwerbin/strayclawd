#include "Canvas.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {

constexpr uint32_t kOrange = 0xD97757;
constexpr uint32_t kInk = 0x1E1E1E;
constexpr uint32_t kPaper = 0xFFFFFF;
constexpr uint32_t kKeyFrame = 0x4A4A4A;
constexpr uint32_t kKeyA = 0xBDBDBD;
constexpr uint32_t kKeyB = 0x9A9A9A;
constexpr uint32_t kKeyLit = 0xFFFFFF;
constexpr uint32_t kKeyBase = 0x6E6E6E;
constexpr uint32_t kScreenGlow = 0x8FD8FF;

struct Glyph {
    char c;
    const char* rows[5];
};

const Glyph kGlyphs[] = {
    {'{', {".XX", ".X.", "X..", ".X.", ".XX"}},
    {'}', {"XX.", ".X.", "..X", ".X.", "XX."}},
    {';', {"...", ".X.", "...", ".X.", "X.."}},
    {'<', {"..X", ".X.", "X..", ".X.", "..X"}},
    {'>', {"X..", ".X.", "..X", ".X.", "X.."}},
    {'/', {"..X", "..X", ".X.", "X..", "X.."}},
    {'=', {"...", "XXX", "...", "XXX", "..."}},
    {'0', {"XXX", "X.X", "X.X", "X.X", "XXX"}},
    {'1', {".X.", "XX.", ".X.", ".X.", "XXX"}},
    {'2', {"XX.", "..X", ".X.", "X..", "XXX"}},
    {'3', {"XX.", "..X", ".X.", "..X", "XX."}},
    {'Z', {"XXX", "..X", ".X.", "X..", "XXX"}},
    {'!', {".X.", ".X.", ".X.", "...", ".X."}},
    {'?', {"XX.", "..X", ".X.", "...", ".X."}},
    {'*', {"...", "X.X", ".X.", "X.X", "..."}},
    {'h', {".X.X.", "XXXXX", "XXXXX", ".XXX.", "..X.."}},
    {'n', {"..XX", "..X.", "..X.", "XXX.", "XXX."}},
    {'d', {".X.", "XXX", "XXX", ".X.", "..."}},
    {'s', {".X.", "XXX", ".X.", "", ""}},
    {'p', {"X", "", "", "", ""}},
    {'c', {"X", "X", "", "", ""}},
    {'t', {"XX", "XX", "", "", ""}},
    {'~', {".X", "X.", ".X", "", ""}},
    {'f', {"..XXXX...X", ".XXXXXX.XX", "XX.XXXXXX.", ".XXXXXX.XX", "..XXXX...X"}},
    {'b', {"..XXXX", "..X.XX", "..XXXX", "XXXXXX", "XXXXXX"}},
    {'k', {"X..........X", "XXXXXXXXXXXX", "..X......X..", "", ""}},
    {'v', {"....X", "...X.", "X.X..", ".X...", ""}},
    {'g', {"X...X", ".XXX.", "XXXXX", ".XXX.", "X...X"}},
};

constexpr uint32_t kNightcap = 0x3B5BA9;
constexpr uint32_t kSanta = 0xD63A3A;
constexpr uint32_t kWitchBand = 0x7B3FA0;
constexpr uint32_t kPartyA = 0xE86FB0;
constexpr uint32_t kPartyB = 0x4FC3E8;
constexpr uint32_t kGlasses = 0xBFBFBF;
constexpr uint32_t kBookCover = 0x2E4A8C;
constexpr uint32_t kGreen = 0x4FA35A;
constexpr uint32_t kGreenLight = 0x7CCB6B;
constexpr uint32_t kGreenDark = 0x2F7A3A;
constexpr uint32_t kBookSpine = 0x1F3566;
constexpr uint32_t kMug = 0x3E7CD8;
constexpr uint32_t kMugRim = 0x8FBCF5;
constexpr uint32_t kUmbrella = 0x3E7CD8;
constexpr uint32_t kPot = 0xC8643C;
constexpr uint32_t kPotDark = 0x9C4A2A;
constexpr uint32_t kPetal = 0xE86FB0;
constexpr uint32_t kPlates = 0x3A3A3A;
constexpr uint32_t kWheel = 0xDDDDDD;

constexpr uint32_t kHatYellow = 0xF2C230;
constexpr uint32_t kHatYellowDark = 0xC99A1A;
constexpr uint32_t kHatYellowLight = 0xFFE27A;
constexpr uint32_t kBeret = 0xA3304A;
constexpr uint32_t kBeretDark = 0x7E2236;
constexpr uint32_t kTopHat = 0x262626;
constexpr uint32_t kTopHatLight = 0x4A4A4A;
constexpr uint32_t kTopHatBand = 0xC8364A;
constexpr uint32_t kToque = 0xFAFAFA;
constexpr uint32_t kToqueShade = 0xD4D4D4;
constexpr uint32_t kWood = 0xB07A3B;
constexpr uint32_t kWoodDark = 0x8A5A28;
constexpr uint32_t kMetal = 0x9A9A9A;
constexpr uint32_t kPanEdge = 0x6A6A6A;
constexpr uint32_t kPanIn = 0x4A4A4A;
constexpr uint32_t kBlank = 0xF4F1EA;
constexpr uint32_t kChuteRed = 0xE8505B;
constexpr uint32_t kString = 0x6A6A6A;
constexpr uint32_t kFrame = 0x2A2A2A;
constexpr uint32_t kScar = 0x9C3B2E;
constexpr uint32_t kScarfRed = 0x8E1B24;
constexpr uint32_t kScarfGold = 0xE8B923;
constexpr uint32_t kBeard = 0xEDEDED;
constexpr uint32_t kBeardShade = 0xC4C4C4;
constexpr uint32_t kWizard = 0x8E949C;
constexpr uint32_t kWizardShade = 0x6C7179;
constexpr uint32_t kWizardBrim = 0x767B83;
constexpr uint32_t kStraw = 0xE0C068;
constexpr uint32_t kStrawDark = 0xB8963E;
constexpr uint32_t kCrystal = 0xD8E8FF;
constexpr uint32_t kRocketShade = 0xC8C8C8;
constexpr uint32_t kPorthole = 0x6EC6FF;
constexpr uint32_t kFlameYellow = 0xFFD84A;
constexpr uint32_t kFlameOrange = 0xF08A24;
constexpr uint32_t kFlameRed = 0xE23B3B;
constexpr uint32_t kFeathers[3] = {0xE8D5B0, 0xC9A27E, 0x9C7451};
constexpr uint32_t kHeadband = 0x3FB8AF;
constexpr uint32_t kHeadbandDark = 0x2A8C85;
constexpr uint32_t kRobe = 0x3B2350;
constexpr uint32_t kRobeShade = 0x2C1A3D;
constexpr uint32_t kRobeHole = 0x120A1A;
constexpr uint32_t kRobeTrim = 0xC9A23A;
constexpr uint32_t kRobeEyes = 0xFFC83A;
constexpr uint32_t kNeon = 0x39FF14;
constexpr uint32_t kNeonDark = 0x1FBF0A;
constexpr uint32_t kBomb = 0x5B6B3A;
constexpr uint32_t kBombDark = 0x404C28;
constexpr uint32_t kBombLight = 0x7E8F55;
constexpr uint32_t kStone = 0x8A827A;
constexpr uint32_t kStoneLight = 0xB0A8A0;
constexpr uint32_t kCandle = 0xF4E8C8;
constexpr uint32_t kSoup = 0xD9822B;
constexpr uint32_t kSoupLight = 0xF2B35A;
constexpr uint32_t kGold = 0xF2C14E;

const int kRing[8][2] = {{-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}};

int ClawRow(Claw c)
{
    switch (c) {
    case Claw::High: return 0;
    case Claw::Up: return 2;
    case Claw::Mid: return 4;
    default: return 6;
    }
}

}

void Canvas::Resize(int logicalW, int logicalH, int scale)
{
    lw_ = logicalW;
    lh_ = logicalH;
    scale_ = std::max(1, scale);
    px_.assign(static_cast<size_t>(PixelWidth()) * PixelHeight(), 0);
}

void Canvas::Clear()
{
    std::fill(px_.begin(), px_.end(), 0u);
}

void Canvas::Put(int x, int y, uint32_t rgb, int alpha)
{
    if (x < 0 || y < 0 || x >= lw_ || y >= lh_ || alpha <= 0) {
        return;
    }
    const uint32_t a = static_cast<uint32_t>(std::min(alpha, 255));
    const uint32_t r = ((rgb >> 16) & 0xFF) * a / 255;
    const uint32_t g = ((rgb >> 8) & 0xFF) * a / 255;
    const uint32_t b = (rgb & 0xFF) * a / 255;
    const uint32_t inv = 255 - a;
    const int pw = PixelWidth();
    for (int dy = 0; dy < scale_; ++dy) {
        uint32_t* row = &px_[static_cast<size_t>(y * scale_ + dy) * pw + static_cast<size_t>(x) * scale_];
        for (int dx = 0; dx < scale_; ++dx) {
            const uint32_t d = row[dx];
            const uint32_t oa = a + ((d >> 24) & 0xFF) * inv / 255;
            const uint32_t orr = r + ((d >> 16) & 0xFF) * inv / 255;
            const uint32_t og = g + ((d >> 8) & 0xFF) * inv / 255;
            const uint32_t ob = b + (d & 0xFF) * inv / 255;
            row[dx] = (oa << 24) | (orr << 16) | (og << 8) | ob;
        }
    }
}

void Canvas::DrawCrab(const Pose& p, int ox, int oy)
{
    int top = 0;
    int bottom = 7;
    int left = 2;
    int right = 13;
    if (p.body == Body::Crouch) {
        top = 1;
        bottom = 8;
    } else if (p.body == Body::Squash) {
        top = 2;
        bottom = 8;
        left = 1;
        right = 14;
    }

    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            Put(ox + x, oy + y, kOrange);
        }
    }

    const bool spread = p.legs == Legs::Spread;
    const int legCols[4] = {spread ? 2 : 3, spread ? 4 : 5, spread ? 11 : 10, spread ? 13 : 12};
    for (int i = 0; i < 4; ++i) {
        int last = kCrabH - 1;
        if ((p.legs == Legs::LiftLeft && i < 2) || (p.legs == Legs::LiftRight && i >= 2)) {
            last = kCrabH - 2;
        }
        for (int y = bottom + 1; y <= last; ++y) {
            Put(ox + legCols[i], oy + y, kOrange);
        }
    }

    if (p.robe) {
        for (int y = top - 2; y <= kCrabH - 1; ++y) {
            const int x0 = y == top - 2 ? 5 : (y == top - 1 ? 3 : 1);
            for (int x = x0; x <= 15 - x0; ++x) {
                Put(ox + x, oy + y, (x + y) % 6 == 0 ? kRobeShade : kRobe);
            }
        }
        for (int x = 6; x <= 9; ++x) {
            Put(ox + x, oy + top - 3, kRobe);
        }
        Put(ox + 8, oy + top - 4, kRobe);
        for (int y = top + 1; y <= top + 4; ++y) {
            for (int x = 3; x <= 12; ++x) {
                Put(ox + x, oy + y, kRobeHole);
            }
        }
        for (int x = 2; x <= 13; ++x) {
            Put(ox + x, oy + top + 6, kRobeTrim);
        }
        Put(ox + 9, oy + top + 7, kRobeTrim);
        Put(ox + 9, oy + top + 8, kRobeTrim);
    }

    if (p.broom) {
        for (int x = -3; x <= 16; ++x) {
            Put(ox + x, oy + 8, x == -3 ? kWoodDark : kWood);
        }
        Put(ox + 16, oy + 8, kScarfRed);
        for (int x = 17; x <= 20; ++x) {
            const uint32_t c = x % 2 ? kStraw : kStrawDark;
            Put(ox + x, oy + 8, c);
            if (x < 20) {
                Put(ox + x, oy + 7, c);
                Put(ox + x, oy + 9, c);
            }
        }
    }

    const uint32_t eyeInk = p.robe ? kRobeEyes : kInk;
    const int ey = top + 2 + p.lookY;
    const int eyeCols[2] = {4 + p.lookX, 11 + p.lookX};
    for (int i = 0; i < 2; ++i) {
        const int ex = eyeCols[i];
        switch (p.eyes) {
        case Eyes::Open:
            Put(ox + ex, oy + ey, eyeInk);
            Put(ox + ex, oy + ey + 1, eyeInk);
            break;
        case Eyes::Closed:
            Put(ox + ex - (i == 0 ? 1 : 0), oy + ey + 1, eyeInk);
            Put(ox + ex + (i == 0 ? 0 : 1), oy + ey + 1, eyeInk);
            break;
        case Eyes::Happy:
            Put(ox + ex, oy + ey, eyeInk);
            Put(ox + ex - 1, oy + ey + 1, eyeInk);
            Put(ox + ex + 1, oy + ey + 1, eyeInk);
            break;
        case Eyes::Surprised: {
            const int c0 = i == 0 ? ex - 1 : ex;
            for (int y = ey - 1; y <= ey + 1; ++y) {
                Put(ox + c0, oy + y, eyeInk);
                Put(ox + c0 + 1, oy + y, eyeInk);
            }
            break;
        }
        case Eyes::Sly: {
            const int in = i == 0 ? 1 : -1;
            Put(ox + ex, oy + ey + 1, eyeInk);
            Put(ox + ex - in, oy + ey + 1, eyeInk);
            Put(ox + ex - in, oy + ey - 1, eyeInk);
            Put(ox + ex, oy + ey, eyeInk);
            break;
        }
        case Eyes::Dizzy: {
            const int cx = ex;
            const int cy = ey + 1;
            const int f = (i == 0 ? p.frame : -p.frame) & 7;
            for (int k = 0; k < 8; ++k) {
                if (k == f || k == ((f + 1) & 7)) {
                    continue;
                }
                Put(ox + cx + kRing[k][0], oy + cy + kRing[k][1], eyeInk);
            }
            break;
        }
        case Eyes::None:
            break;
        }
    }

    const int hb = top - p.hatDrop;
    auto row = [&](int y, int x0, int x1, uint32_t c) {
        for (int x = x0; x <= x1; ++x) {
            Put(ox + x, oy + y, c);
        }
    };
    switch (p.hat) {
    case Hat::HardHat:
        row(hb - 5, 6, 9, kHatYellow);
        row(hb - 4, 5, 10, kHatYellow);
        row(hb - 3, 4, 11, kHatYellow);
        row(hb - 2, 4, 11, kHatYellow);
        for (int y = hb - 5; y <= hb - 2; ++y) {
            Put(ox + 7, oy + y, kHatYellowLight);
            Put(ox + 8, oy + y, kHatYellowDark);
        }
        Put(ox + 6, oy + hb - 4, kHatYellowLight);
        Put(ox + 5, oy + hb - 3, kHatYellowLight);
        row(hb - 1, 2, 13, kHatYellowDark);
        row(hb - 1, 4, 11, kHatYellow);
        break;
    case Hat::Beret:
        Put(ox + 8, oy + hb - 3, kBeretDark);
        row(hb - 2, 5, 12, kBeret);
        row(hb - 1, 4, 12, kBeretDark);
        break;
    case Hat::TopHat:
        for (int y = hb - 6; y <= hb - 3; ++y) {
            row(y, 5, 10, kTopHat);
            Put(ox + 6, oy + y, kTopHatLight);
        }
        row(hb - 2, 5, 10, kTopHatBand);
        row(hb - 1, 3, 12, kTopHat);
        break;
    case Hat::Toque:
        row(hb - 7, 5, 10, kToque);
        row(hb - 6, 4, 11, kToque);
        row(hb - 5, 4, 11, kToque);
        row(hb - 4, 5, 10, kToque);
        Put(ox + 7, oy + hb - 6, kToqueShade);
        Put(ox + 9, oy + hb - 5, kToqueShade);
        row(hb - 3, 5, 10, kToque);
        row(hb - 2, 5, 10, kToque);
        row(hb - 1, 5, 10, kToqueShade);
        break;
    case Hat::Nightcap:
    case Hat::Santa: {
        const uint32_t c = p.hat == Hat::Santa ? kSanta : kNightcap;
        row(hb - 4, 9, 13, c);
        row(hb - 3, 6, 14, c);
        row(hb - 2, 4, 11, c);
        Put(ox + 15, oy + hb - 2, kPaper);
        Put(ox + 15, oy + hb - 1, kPaper);
        Put(ox + 14, oy + hb - 2, c);
        row(hb - 1, 3, 12, kPaper);
        if (p.hat == Hat::Nightcap) {
            Put(ox + 7, oy + hb - 2, kToqueShade);
            Put(ox + 10, oy + hb - 3, kToqueShade);
        }
        break;
    }
    case Hat::Witch:
        Put(ox + 9, oy + hb - 7, kTopHat);
        Put(ox + 8, oy + hb - 6, kTopHat);
        row(hb - 5, 7, 8, kTopHat);
        row(hb - 4, 6, 8, kTopHat);
        row(hb - 3, 6, 9, kTopHat);
        row(hb - 2, 5, 10, kWitchBand);
        row(hb - 1, 2, 13, kTopHat);
        break;
    case Hat::Wizard:
        row(hb - 1, 0, 15, kWizardBrim);
        row(hb - 2, 4, 11, kWizard);
        row(hb - 3, 5, 10, kWizard);
        row(hb - 4, 5, 9, kWizard);
        row(hb - 5, 6, 9, kWizard);
        row(hb - 6, 7, 9, kWizard);
        row(hb - 7, 8, 9, kWizard);
        row(hb - 8, 9, 10, kWizard);
        Put(ox + 11, oy + hb - 9, kWizard);
        for (int y = hb - 5; y <= hb - 2; ++y) {
            Put(ox + (y >= hb - 3 ? 5 : 6), oy + y, kWizardShade);
        }
        Put(ox + 4, oy + hb - 2, kWizardShade);
        break;
    case Hat::Headband:
        row(hb, 2, 13, kHeadband);
        Put(ox + 8, oy + hb, kHeadbandDark);
        Put(ox + 14, oy + hb + 1, kHeadband);
        Put(ox + 15, oy + hb + 2, kHeadbandDark);
        Put(ox + 14, oy + hb + 2, kHeadband);
        break;
    case Hat::Party:
        row(hb - 6, 7, 8, kHatYellow);
        row(hb - 5, 7, 8, kPartyA);
        row(hb - 4, 7, 8, kPartyB);
        row(hb - 3, 6, 9, kPartyA);
        row(hb - 2, 6, 9, kPartyB);
        row(hb - 1, 5, 10, kPartyA);
        break;
    case Hat::None:
        break;
    }

    if (p.mouth) {
        row(top + 4, 7, 8, kInk);
        row(top + 5, 7, 8, kInk);
    }
    if (p.glasses) {
        const int gy = top + 2 + p.lookY;
        for (int i = 0; i < 2; ++i) {
            const int ex = (i == 0 ? 4 : 11) + p.lookX;
            for (int y = gy - 1; y <= gy + 2; ++y) {
                Put(ox + ex - 1, oy + y, kGlasses);
                Put(ox + ex + 1, oy + y, kGlasses);
            }
            Put(ox + ex, oy + gy - 1, kGlasses);
            Put(ox + ex, oy + gy + 2, kGlasses);
        }
        row(gy, 6, 9, kGlasses);
    }
    if (p.roundGlasses) {
        const int gy = top + 2 + p.lookY;
        for (int i = 0; i < 2; ++i) {
            const int ex = (i == 0 ? 4 : 11) + p.lookX;
            for (int y = gy; y <= gy + 1; ++y) {
                Put(ox + ex - 2, oy + y, kFrame);
                Put(ox + ex + 2, oy + y, kFrame);
                Put(ox + ex - 1, oy + y, kPaper, 110);
                Put(ox + ex + 1, oy + y, kPaper, 110);
            }
            row(gy - 1, ex - 1, ex + 1, kFrame);
            row(gy + 2, ex - 1, ex + 1, kFrame);
        }
        row(gy, 7 + p.lookX, 8 + p.lookX, kFrame);
    }
    if (p.scar) {
        Put(ox + 9, oy + top, kScar);
        Put(ox + 8, oy + top + 1, kScar);
    }
    if (p.beard) {
        for (int i = 0; i < 2; ++i) {
            const int ex = (i == 0 ? 4 : 11) + p.lookX;
            row(ey - 1, ex - 1, ex + 1, kBeard);
        }
        row(top + 4, 6, 9, kBeard);
        row(top + 5, 3, 12, kBeard);
        row(top + 6, 3, 12, kBeard);
        row(top + 7, 4, 11, kBeard);
        row(top + 8, 5, 10, kBeard);
        row(top + 9, 6, 9, kBeard);
        Put(ox + 5, oy + top + 6, kBeardShade);
        Put(ox + 10, oy + top + 6, kBeardShade);
        Put(ox + 7, oy + top + 7, kBeardShade);
        Put(ox + 8, oy + top + 8, kBeardShade);
    }
    if (p.scarf) {
        const int sy = top + 5;
        for (int x = 2; x <= 13; ++x) {
            Put(ox + x, oy + sy, (x / 2) % 2 ? kScarfGold : kScarfRed);
        }
        for (int y = sy + 1; y <= sy + 3; ++y) {
            const uint32_t c = (y - sy) % 2 ? kScarfGold : kScarfRed;
            Put(ox + 10, oy + y, c);
            Put(ox + 11, oy + y, c);
        }
    }

    if (p.keyboard) {
        for (int x = 1; x <= 14; ++x) {
            Put(ox + x, oy + 7, kKeyFrame);
            uint32_t key = (x == 1 || x == 14) ? kKeyFrame : ((x % 2) ? kKeyA : kKeyB);
            if (x == p.pressedKey) {
                key = kKeyLit;
            }
            Put(ox + x, oy + 8, key);
            Put(ox + x, oy + 9, kKeyBase);
        }
    }

    if (p.book >= 0) {
        for (int y = top + 5; y <= top + 8; ++y) {
            row(y, 4, 11, kBookCover);
            Put(ox + 7, oy + y, kBookSpine);
            Put(ox + 8, oy + y, kBookSpine);
        }
        Put(ox + 4, oy + top + 4, kBookCover);
        row(top + 4, 5, 6, kBlank);
        row(top + 4, 9, 10, kBlank);
        Put(ox + 11, oy + top + 4, kBookCover);
        row(top + 6, 9, 10, kHatYellow);
        Put(ox + 10, oy + top + 7, kHatYellow);
        const int flip = p.book % 4;
        if (flip > 0) {
            Put(ox + 11 - 2 * flip, oy + top + 3, kPaper);
            Put(ox + 10 - 2 * flip, oy + top + 3, kPaper);
        }
    }
    if (p.skate) {
        row(10, 0, 15, kWood);
        Put(ox + 0, oy + 9, kWood);
        Put(ox + 15, oy + 9, kWood);
        row(11, 2, 3, kWheel);
        row(11, 12, 13, kWheel);
    }

    const int lr = top + ClawRow(p.left);
    const int rr = top + ClawRow(p.right);
    for (int d = 0; d < 2; ++d) {
        Put(ox + 0, oy + lr + d, kOrange);
        Put(ox + 1, oy + lr + d, kOrange);
        Put(ox + 14, oy + rr + d, kOrange);
        Put(ox + 15, oy + rr + d, kOrange);
    }

    const int side = p.toolSide < 0 ? -1 : 1;
    const int r = side > 0 ? rr : lr;
    auto tp = [&](int x, int y, uint32_t c, int alpha = 255) { Put(ox + (side > 0 ? x : 15 - x), oy + y, c, alpha); };
    switch (p.tool) {
    case Tool::Hammer:
        for (int y = r - 3; y <= r - 1; ++y) {
            tp(15, y, kWoodDark);
        }
        for (int x = 14; x <= 17; ++x) {
            tp(x, r - 5, kMetal);
            tp(x, r - 4, kMetal);
        }
        break;
    case Tool::Wrench:
        tp(15, r - 1, kChuteRed);
        tp(16, r - 2, kChuteRed);
        tp(16, r - 3, kMetal);
        tp(17, r - 4, kMetal);
        tp(18, r - 4, kPanEdge);
        tp(16, r - 5, kMetal);
        tp(16, r - 6, kMetal);
        tp(19, r - 5, kMetal);
        tp(19, r - 6, kMetal);
        tp(17, r - 5, kPanEdge);
        break;
    case Tool::Brush:
        for (int y = r - 3; y <= r; ++y) {
            tp(16, y, kWood);
        }
        tp(16, r - 4, p.brushColor);
        tp(16, r - 5, p.brushColor);
        break;
    case Tool::Wand:
        for (int y = r - 4; y <= r; ++y) {
            tp(16, y, p.woodWand ? kWoodDark : kTopHat);
        }
        tp(16, r - 5, kPaper);
        break;
    case Tool::Pan: {
        const int b = r + 1 - p.toolLift;
        tp(16, r + 1, kWoodDark);
        tp(17, p.toolLift > 0 ? r : r + 1, kWoodDark);
        for (int x = 18; x <= 25; ++x) {
            tp(x, b, kMetal);
            tp(x, b + 1, kPanEdge);
        }
        for (int x = 19; x <= 24; ++x) {
            tp(x, b + 2, kPanIn);
        }
        break;
    }
    case Tool::Rod:
        for (int k = 0; k <= 4; ++k) {
            tp(16 + k, r - k, kWoodDark);
        }
        break;
    case Tool::Mug:
        for (int x = 16; x <= 19; ++x) {
            tp(x, r - 1, kMugRim);
            for (int y = r; y <= r + 2; ++y) {
                tp(x, y, kMug);
            }
        }
        tp(17, r + 1, kPaper);
        tp(20, r, kMug);
        tp(21, r, kMug);
        tp(21, r + 1, kMug);
        tp(21, r + 2, kMug);
        tp(20, r + 2, kMug);
        break;
    case Tool::Can:
        for (int y = r; y <= r + 2; ++y) {
            for (int x = 16; x <= 19; ++x) {
                tp(x, y, kGreen);
            }
        }
        tp(17, r - 1, kGreenDark);
        tp(18, r - 1, kGreenDark);
        tp(20, r, kGreen);
        tp(21, r - 1, kGreen);
        tp(22, r - 2, kGreen);
        break;
    case Tool::Umbrella: {
        for (int y = top - 4; y <= r - 1; ++y) {
            tp(14, y, kInk);
        }
        const int widths[3] = {8, 10, 12};
        for (int j = 0; j < 3; ++j) {
            for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
                if (j == 2 && (dx + 12) % 4 == 3) {
                    continue;
                }
                const uint32_t c = ((dx + 24) / 3) % 2 ? kUmbrella : kPaper;
                tp(12 + dx, top - 7 + j, c);
            }
        }
        tp(12, top - 8, kInk);
        break;
    }
    case Tool::Duster: {
        for (int y = r - 3; y <= r; ++y) {
            tp(17, y, kWoodDark);
        }
        tp(15, r - 9, kFeathers[0]);
        tp(17, r - 9, kFeathers[1]);
        tp(19, r - 9, kFeathers[0]);
        const int tuft[5][2] = {{15, 19}, {14, 20}, {14, 20}, {15, 19}, {16, 18}};
        for (int j = 0; j < 5; ++j) {
            for (int x = tuft[j][0]; x <= tuft[j][1]; ++x) {
                tp(x, r - 8 + j, kFeathers[(x + j) % 3]);
            }
        }
        break;
    }
    case Tool::Broom: {
        const int lift = p.toolLift;
        const int y0 = r - 3;
        const int x1 = 19 + lift;
        const int n = 7 - y0;
        for (int k = 0; k <= n; ++k) {
            const int x = 15 + static_cast<int>(std::lround(static_cast<double>(x1 - 15) * k / n));
            tp(x, y0 + k, kWood);
        }
        for (int x = 18; x <= 21; ++x) {
            tp(x + lift, 7, kWoodDark);
        }
        for (int x = 17; x <= 22; ++x) {
            tp(x + lift, 8, x % 2 ? kStraw : kStrawDark);
            tp(x + lift, 9, x % 2 ? kStrawDark : kStraw);
        }
        break;
    }
    case Tool::Staff: {
        for (int y = r - 7; y <= kCrabH - 1; ++y) {
            tp(16, y, (y - r) % 4 == 0 ? kWoodDark : kWood);
        }
        tp(15, r - 8, kWoodDark);
        tp(17, r - 8, kWoodDark);
        tp(16, r - 9, kWoodDark);
        tp(16, r - 8, p.glow > 0 ? kPaper : kCrystal);
        if (p.glow > 0) {
            for (int k = 0; k < 8; ++k) {
                tp(16 + 2 * kRing[k][0], r - 8 + 2 * kRing[k][1], kCrystal, p.glow / 3);
            }
        }
        break;
    }
    case Tool::Saber: {
        tp(16, r - 1, kMetal);
        tp(16, r, kPanEdge);
        tp(16, r + 1, kMetal);
        const uint32_t core = ((p.saberColor >> 1) & 0x7F7F7F) + 0x808080;
        for (int pass = 0; pass < 2; ++pass) {
            for (int k = 0; k < p.saberLen; ++k) {
                int bx = 16;
                int by = r - 2 - k;
                if (p.saberAngle == 1) {
                    bx = 17 + k;
                } else if (p.saberAngle == 2) {
                    bx = 17 + k;
                    by = r - 1;
                }
                if (pass == 0) {
                    for (int j = 0; j < 4; ++j) {
                        tp(bx + kRing[j * 2 + 1][0], by + kRing[j * 2 + 1][1], p.saberColor, 110);
                    }
                } else {
                    tp(bx, by, core);
                }
            }
        }
        break;
    }
    case Tool::Paddle:
        tp(16, r, kWoodDark);
        tp(16, r - 1, kWoodDark);
        for (int y = r - 5; y <= r - 2; ++y) {
            for (int x = 15; x <= 18; ++x) {
                const bool corner = (y == r - 5 || y == r - 2) && (x == 15 || x == 18);
                if (!corner) {
                    tp(x, y, x == 18 || y == r - 2 ? kFlameRed : kChuteRed);
                }
            }
        }
        break;
    case Tool::Phone:
        for (int y = r - 5; y <= r; ++y) {
            tp(16, y, kInk);
            tp(17, y, kInk);
        }
        tp(16, r - 4, kScreenGlow);
        tp(16, r - 3, kScreenGlow);
        tp(17, r - 4, kPorthole);
        break;
    case Tool::Pillow:
        for (int y = r - 4; y <= r; ++y) {
            for (int x = 15; x <= 21; ++x) {
                const bool corner = (y == r - 4 || y == r) && (x == 15 || x == 21);
                if (!corner) {
                    tp(x, y, (y == r || x == 21) ? kToqueShade : kPaper);
                }
            }
        }
        tp(18, r - 2, kPorthole);
        break;
    case Tool::Sandwich: {
        const int w = std::max(0, 4 - p.toolLift);
        for (int x = 16; x < 16 + w; ++x) {
            tp(x, r - 1, kStraw);
            tp(x, r, x % 2 ? kGreen : kChuteRed);
            tp(x, r + 1, kStrawDark);
        }
        break;
    }
    case Tool::Guitar: {
        const int strum = p.toolLift;
        for (int k = 0; k < 6; ++k) {
            tp(15 + k, 3 - k, kWoodDark);
        }
        tp(20, -3, kInk);
        tp(21, -3, kInk);
        const int body[4][2] = {{9, 13}, {8, 14}, {8, 14}, {9, 13}};
        for (int j = 0; j < 4; ++j) {
            for (int x = body[j][0]; x <= body[j][1]; ++x) {
                tp(x, 4 + j, j == 3 ? kWoodDark : kWood);
            }
        }
        tp(11, 5, kInk);
        tp(11, 6, kInk);
        if (strum) {
            tp(12, 4, kPaper);
        }
        break;
    }
    case Tool::Mic:
        tp(16, r + 1, kPanEdge);
        tp(16, r, kPanEdge);
        tp(16, r - 1, kInk);
        for (int y = r - 3; y <= r - 2; ++y) {
            tp(15, y, kMetal);
            tp(16, y, kPanEdge);
            tp(17, y, kMetal);
        }
        break;
    case Tool::None:
        break;
    }

    if (p.barbell != kNoBarbell) {
        const int by = top + p.barbell;
        for (int x = -3; x <= 18; ++x) {
            Put(ox + x, oy + by, kMetal);
        }
        for (int y = by - 1; y <= by + 1; ++y) {
            for (int x : {-3, -2, 17, 18}) {
                Put(ox + x, oy + y, kPlates);
            }
        }
    }
}

void Canvas::DrawLine(int x0, int y0, int x1, int y1, uint32_t rgb, int alpha)
{
    const int dx = std::abs(x1 - x0);
    const int dy = -std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        Put(x0, y0, rgb, alpha);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Canvas::DrawPlant(int x, int bottom, int level)
{
    for (int y = bottom - 2; y <= bottom; ++y) {
        for (int i = 0; i <= 4; ++i) {
            Put(x + i, y, y == bottom ? kPotDark : kPot);
        }
    }
    for (int i = -1; i <= 5; ++i) {
        Put(x + i, bottom - 3, kPotDark);
    }
    const int h = level * 2;
    for (int k = 0; k < h; ++k) {
        const int y = bottom - 4 - k;
        Put(x + 2, y, kGreen);
        if (k % 2 == 1) {
            Put(x + ((k / 2) % 2 ? 1 : 3), y, kGreenLight);
        }
    }
    if (level >= 5) {
        const int fy = bottom - 4 - h;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = 1; dx <= 3; ++dx) {
                Put(x + dx, fy + dy, kPetal);
            }
        }
        Put(x + 2, fy, kHatYellow);
        Put(x + 2, fy - 2, kPetal);
        Put(x, fy, kPetal);
        Put(x + 4, fy, kPetal);
    } else if (level >= 4) {
        const int fy = bottom - 4 - h;
        Put(x + 2, fy, kHatYellow);
        Put(x + 1, fy, kPetal);
        Put(x + 3, fy, kPetal);
        Put(x + 2, fy - 1, kPetal);
        Put(x + 2, fy + 1, kPetal);
    }
}

void Canvas::DrawRainbow(int cx, int cy, int alpha)
{
    const uint32_t colors[5] = {0xE8505B, 0xF08A24, 0xF2C14E, 0x5DBB4A, 0x3E7CD8};
    for (int y = cy - 21; y <= cy; ++y) {
        for (int x = cx - 21; x <= cx + 21; ++x) {
            const double d = std::sqrt(static_cast<double>((x - cx) * (x - cx) + (y - cy) * (y - cy)));
            const int band = static_cast<int>(20.5 - d);
            if (d < 20.5 && band >= 0 && band < 5) {
                Put(x, y, colors[band], alpha);
            }
        }
    }
}

void Canvas::DrawRocket(int x, int y, int level, int flame)
{
    const int minRows[5] = {20, 13, 9, 6, 0};
    const int minRow = minRows[std::clamp(level, 0, 4)];
    auto put = [&](int c, int r, uint32_t rgb) {
        if (r >= minRow) {
            Put(x + c, y + r, rgb);
        }
    };
    const int nose[6][2] = {{6, 7}, {5, 8}, {4, 9}, {4, 9}, {3, 10}, {3, 10}};
    for (int r = 0; r < 6; ++r) {
        for (int c = nose[r][0]; c <= nose[r][1]; ++c) {
            put(c, r, kChuteRed);
        }
    }
    for (int r = 6; r <= 17; ++r) {
        for (int c = 3; c <= 10; ++c) {
            put(c, r, c == 10 ? kRocketShade : kBlank);
        }
    }
    put(6, 8, kMetal);
    put(7, 8, kMetal);
    put(5, 9, kMetal);
    put(6, 9, kPorthole);
    put(7, 9, kPorthole);
    put(8, 9, kMetal);
    put(6, 10, kMetal);
    put(7, 10, kMetal);
    for (int c = 3; c <= 10; ++c) {
        put(c, 13, kChuteRed);
    }
    put(2, 13, kChuteRed);
    put(11, 13, kChuteRed);
    for (int r = 14; r <= 18; ++r) {
        const int w = r == 14 ? 1 : 2;
        for (int k = 0; k < w + 1; ++k) {
            put(2 - k, r, kChuteRed);
            put(11 + k, r, kChuteRed);
        }
    }
    for (int c = 4; c <= 9; ++c) {
        put(c, 18, kPanEdge);
    }
    for (int c = 5; c <= 8; ++c) {
        put(c, 19, kPanIn);
    }
    if (flame <= 0) {
        return;
    }
    const int len = flame % 2 ? 4 : 3;
    const uint32_t colors[4] = {kFlameYellow, kFlameYellow, kFlameOrange, kFlameRed};
    const int widths[4] = {3, 2, 1, 1};
    for (int j = 0; j < len; ++j) {
        for (int c = 7 - widths[j]; c <= 6 + widths[j]; ++c) {
            Put(x + c, y + 20 + j, colors[j]);
        }
    }
}

void Canvas::DrawLadder(int x, int bottom, int height)
{
    for (int y = bottom - height + 1; y <= bottom; ++y) {
        Put(x, y, kWood);
        Put(x + 5, y, kWood);
        if ((bottom - y) % 3 == 1) {
            for (int i = 1; i <= 4; ++i) {
                Put(x + i, y, kWoodDark);
            }
        }
    }
}

void Canvas::DrawEasel(int x, int y, const uint32_t* painting, const bool* revealed)
{
    for (int j = 0; j <= 6; ++j) {
        for (int i = 0; i <= 8; ++i) {
            if (i == 0 || i == 8 || j == 0 || j == 6) {
                Put(x + i, y + j, kWoodDark);
            } else {
                const int idx = (j - 1) * 7 + (i - 1);
                Put(x + i, y + j, revealed[idx] ? painting[idx] : kBlank);
            }
        }
    }
    const int legs[3][3] = {{2, 1, 0}, {6, 7, 8}, {4, 4, 4}};
    for (int k = 0; k < 3; ++k) {
        Put(x + legs[k][0], y + 7, kWood);
        Put(x + legs[k][1], y + 8, kWood);
        Put(x + legs[k][1], y + 9, kWood);
        Put(x + legs[k][2], y + 10, kWood);
    }
}

void Canvas::DrawParachute(int cx, int top, int fold)
{
    auto stripe = [&](int dx) { return ((dx + 30) / 3) % 2 ? kChuteRed : kHatYellow; };
    if (fold == 0) {
        const int widths[8] = {6, 9, 11, 12, 13, 14, 14, 14};
        for (int j = 0; j < 8; ++j) {
            for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
                if (j == 7 && (dx + 14) % 7 == 3) {
                    continue;
                }
                Put(cx + dx, top - 5 + j, stripe(dx));
            }
        }
        const int from[4] = {-14, -5, 5, 14};
        const int to[4] = {-8, -4, 3, 7};
        for (int k = 0; k < 4; ++k) {
            for (int y = top + 3; y <= top + 13; ++y) {
                const double t = (y - (top + 3)) / 10.0;
                const int x = cx + static_cast<int>(std::lround(from[k] + (to[k] - from[k]) * t));
                Put(x, y, kString, 220);
            }
        }
    } else if (fold == 1) {
        const int widths[4] = {6, 9, 11, 12};
        for (int j = 0; j < 4; ++j) {
            for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
                Put(cx + dx, top + 6 + j, stripe(dx));
            }
        }
    } else {
        for (int dx = -14; dx <= 14; ++dx) {
            Put(cx + dx, top + 22, stripe(dx));
        }
        for (int dx = -15; dx <= 15; ++dx) {
            Put(cx + dx, top + 23, stripe(dx));
        }
    }
}

void Canvas::DrawGlyph(char c, int x, int y, uint32_t rgb, int alpha)
{
    for (const Glyph& g : kGlyphs) {
        if (g.c != c) {
            continue;
        }
        for (int pass = 0; pass < 2; ++pass) {
            for (int r = 0; r < 5; ++r) {
                for (int j = 0; g.rows[r][j] != '\0'; ++j) {
                    if (g.rows[r][j] != 'X') {
                        continue;
                    }
                    if (pass == 0) {
                        Put(x + j + 1, y + r + 1, 0x000000, alpha * 2 / 5);
                    } else {
                        Put(x + j, y + r, rgb, alpha);
                    }
                }
            }
        }
        return;
    }
}

void Canvas::DrawBubble(int x, int y, int dots)
{
    for (int j = 0; j < 7; ++j) {
        for (int i = 0; i < 9; ++i) {
            const bool edgeX = i == 0 || i == 8;
            const bool edgeY = j == 0 || j == 6;
            if (edgeX && edgeY) {
                continue;
            }
            Put(x + i, y + j, (edgeX || edgeY) ? kInk : kPaper);
        }
    }
    for (int d = 0; d < dots && d < 3; ++d) {
        Put(x + 2 + 2 * d, y + 3, kInk);
    }
    Put(x + 1, y + 8, kInk);
    Put(x, y + 10, kInk);
}

namespace {

struct TextGlyph {
    char c;
    const char* rows[5];
};

const TextGlyph kFont[] = {
    {'T', {"XXX", ".X.", ".X.", ".X.", ".X."}},
    {'H', {"X.X", "X.X", "XXX", "X.X", "X.X"}},
    {'I', {"X", "X", "X", "X", "X"}},
    {'S', {".XX", "X..", ".X.", "..X", "XX."}},
    {'F', {"XXX", "X..", "XX.", "X..", "X.."}},
    {'N', {"X..X", "XX.X", "X.XX", "X..X", "X..X"}},
    {'E', {"XXX", "X..", "XX.", "X..", "XXX"}},
    {'.', {".", ".", ".", ".", "X"}},
    {'!', {"X", "X", "X", ".", "X"}},
    {'?', {"XX.", "..X", ".X.", "...", ".X."}},
    {' ', {"..", "..", "..", "..", ".."}},
    {'B', {"XX.", "X.X", "XX.", "X.X", "XX."}},
    {'O', {".X.", "X.X", "X.X", "X.X", ".X."}},
    {'M', {"X...X", "XX.XX", "X.X.X", "X...X", "X...X"}},
    {'L', {"X..", "X..", "X..", "X..", "XXX"}},
    {'G', {".XX", "X..", "X.X", "X.X", ".XX"}},
    {'D', {"XX.", "X.X", "X.X", "X.X", "XX."}},
    {'P', {"XX.", "X.X", "XX.", "X..", "X.."}},
    {'Y', {"X.X", "X.X", ".X.", ".X.", ".X."}},
};

const TextGlyph* FindText(char c)
{
    for (const TextGlyph& g : kFont) {
        if (g.c == c) {
            return &g;
        }
    }
    return nullptr;
}

}

int Canvas::TextWidth(const char* text)
{
    int w = 0;
    for (const char* p = text; *p; ++p) {
        if (const TextGlyph* g = FindText(*p)) {
            w += static_cast<int>(std::strlen(g->rows[0])) + 1;
        }
    }
    return std::max(0, w - 1);
}

void Canvas::DrawText(const char* text, int x, int y, uint32_t rgb, int visible)
{
    int n = 0;
    for (const char* p = text; *p && n < visible; ++p, ++n) {
        const TextGlyph* g = FindText(*p);
        if (!g) {
            continue;
        }
        for (int r = 0; r < 5; ++r) {
            for (int j = 0; g->rows[r][j] != '\0'; ++j) {
                if (g->rows[r][j] == 'X') {
                    Put(x + j, y + r, rgb);
                }
            }
        }
        x += static_cast<int>(std::strlen(g->rows[0])) + 1;
    }
}

void Canvas::DrawBigText(const char* text, int cx, int y, uint32_t fill, uint32_t outline, int alpha)
{
    const int x0 = cx - TextWidth(text);
    for (int pass = 0; pass < 2; ++pass) {
        int x = x0;
        for (const char* p = text; *p; ++p) {
            const TextGlyph* g = FindText(*p);
            if (!g) {
                continue;
            }
            for (int r = 0; r < 5; ++r) {
                for (int j = 0; g->rows[r][j] != '\0'; ++j) {
                    if (g->rows[r][j] != 'X') {
                        continue;
                    }
                    for (int k = 0; k < 4; ++k) {
                        const int px = x + 2 * j + k % 2;
                        const int py = y + 2 * r + k / 2;
                        if (pass == 0) {
                            for (int o = 0; o < 8; ++o) {
                                Put(px + kRing[o][0], py + kRing[o][1], outline, alpha);
                            }
                        } else {
                            Put(px, py, r < 2 ? kPaper : fill, alpha);
                        }
                    }
                }
            }
            x += 2 * (static_cast<int>(std::strlen(g->rows[0])) + 1);
        }
    }
}

void Canvas::DrawSpeech(int cx, int bottom, const char* text, int visible)
{
    const int w = TextWidth(text) + 6;
    const int x0 = std::clamp(cx - w / 2, 0, std::max(0, lw_ - w));
    const int y0 = bottom - 11;
    for (int j = 0; j < 9; ++j) {
        for (int i = 0; i < w; ++i) {
            const bool edgeX = i == 0 || i == w - 1;
            const bool edgeY = j == 0 || j == 8;
            if (edgeX && edgeY) {
                continue;
            }
            Put(x0 + i, y0 + j, (edgeX || edgeY) ? kInk : kPaper);
        }
    }
    const int tx = std::clamp(cx, x0 + 2, x0 + w - 4);
    Put(tx, y0 + 8, kPaper);
    Put(tx + 1, y0 + 8, kPaper);
    Put(tx - 1, y0 + 9, kInk);
    Put(tx, y0 + 9, kPaper);
    Put(tx + 1, y0 + 9, kInk);
    Put(tx, y0 + 10, kInk);
    DrawText(text, x0 + 3, y0 + 2, kInk, visible);
}

void Canvas::DrawSign(int cx, int bottom)
{
    for (int y = bottom - 7; y <= bottom; ++y) {
        Put(cx, y, kWoodDark);
    }
    const int top = bottom - 16;
    for (int j = 0; j < 9; ++j) {
        const int hw = (j + 1) / 2;
        for (int dx = -hw; dx <= hw; ++dx) {
            const bool edge = dx == -hw || dx == hw || j == 8;
            Put(cx + dx, top + j, edge ? kInk : kHatYellow);
        }
    }
    for (int j : {3, 4, 5, 7}) {
        Put(cx, top + j, kInk);
    }
}

void Canvas::DrawBigRocket(int cx, int bottom, int level, int flame)
{
    const int minRows[6] = {56, 44, 33, 22, 10, 0};
    const int minRow = minRows[std::clamp(level, 0, 5)];
    const int top = bottom - 55;
    auto put = [&](int dx, int r, uint32_t rgb) {
        if (r >= minRow) {
            Put(cx + dx, top + r, rgb);
        }
    };
    auto white = [&](int dx, int half) { return dx == half ? kRocketShade : kBlank; };
    for (int r = 0; r <= 2; ++r) {
        put(0, r, r == 0 ? kChuteRed : kPanEdge);
    }
    for (int r = 3; r <= 5; ++r) {
        put(-1, r, (r % 2) ? kPanEdge : kInk);
        put(0, r, kPanEdge);
        put(1, r, (r % 2) ? kInk : kPanEdge);
    }
    const int cone[4] = {1, 1, 2, 3};
    for (int r = 6; r <= 9; ++r) {
        const int half = cone[r - 6];
        for (int dx = -half; dx <= half; ++dx) {
            put(dx, r, white(dx, half));
        }
    }
    for (int r = 10; r <= 53; ++r) {
        int half = 5;
        if (r <= 10) {
            half = 3;
        } else if (r <= 21) {
            half = 4;
        }
        for (int dx = -half; dx <= half; ++dx) {
            uint32_t c = white(dx, half);
            if (r == 20 || r == 21 || r == 22 || r == 32) {
                c = kInk;
            }
            const bool checkA = r >= 33 && r <= 36;
            const bool checkB = r >= 50 && r <= 53;
            if (checkA || checkB) {
                const bool q = dx <= -3 || (dx >= 1 && dx <= 3);
                c = (q == checkA) ? kInk : white(dx, half);
            }
            put(dx, r, c);
        }
    }
    const char* usa[3][3] = {{"X.X", "X.X", "XXX"}, {".XX", ".X.", "XX."}, {".X.", "XXX", "X.X"}};
    for (int k = 0; k < 3; ++k) {
        for (int j = 0; j < 3; ++j) {
            for (int i = 0; i < 3; ++i) {
                if (usa[k][j][i] == 'X') {
                    put(i - 2, 39 + k * 4 + j, kInk);
                }
            }
        }
    }
    for (int j = 0; j < 3; ++j) {
        put(2, 24 + j, kChuteRed);
        put(3, 24 + j, j == 0 ? kPorthole : kChuteRed);
    }
    for (int r = 46; r <= 55; ++r) {
        const int w = std::min(4, 1 + (r - 46) / 2);
        for (int k = 1; k <= w; ++k) {
            put(-5 - k, r, k == w ? kInk : kPanEdge);
            put(5 + k, r, k == w ? kInk : kPanEdge);
        }
    }
    for (int r = 54; r <= 55; ++r) {
        for (int dx = -5; dx <= 5; ++dx) {
            put(dx, r, ((dx + 5) % 2 == 0) == (r == 55) ? kPanIn : kPanEdge);
        }
    }
    if (flame <= 0) {
        return;
    }
    const int len = flame % 2 ? 10 : 9;
    const uint32_t colors[10] = {kFlameYellow, kFlameYellow, kFlameYellow, kFlameYellow, kFlameOrange,
                                 kFlameOrange, kFlameOrange, kFlameRed, kFlameRed, kFlameRed};
    const int widths[10] = {5, 6, 6, 5, 5, 4, 3, 2, 2, 1};
    for (int j = 0; j < len; ++j) {
        for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
            Put(cx + dx, bottom + 1 + j, colors[j]);
        }
    }
}

void Canvas::DrawPot(int cx, int bottom, int frame)
{
    Put(cx - 4, bottom, kPanEdge);
    Put(cx + 4, bottom, kPanEdge);
    for (int y = bottom - 5; y <= bottom - 1; ++y) {
        const int hw = y == bottom - 1 ? 4 : 5;
        for (int dx = -hw; dx <= hw; ++dx) {
            Put(cx + dx, y, (dx == -hw + 1 && y < bottom - 2) ? kPanEdge : kPanIn);
        }
    }
    for (int dx = -6; dx <= 6; ++dx) {
        Put(cx + dx, bottom - 6, (dx == -6 || dx == 6) ? kPanEdge : kSoup);
    }
    Put(cx - 2 + (frame / 2) % 5, bottom - 6, kSoupLight);
    Put(cx + 3 - (frame / 3) % 6, bottom - 6, kSoupLight);
    const int stir[8] = {-2, -1, 0, 1, 2, 1, 0, -1};
    const int sx = cx + stir[(frame / 2) % 8];
    for (int k = 0; k < 6; ++k) {
        Put(sx + k / 2, bottom - 7 - k, kWoodDark);
    }
}

void Canvas::DrawBomb(int cx, int bottom, int frame, int puddle)
{
    const int bx = cx + 3;
    const int by = bottom - 14;
    const int px = bx + 8;
    const int ph = puddle * 3;
    for (int dx = -ph; dx <= ph; ++dx) {
        Put(px + dx, bottom, ((dx + frame / 3) % 5 == 0) ? kNeon : kNeonDark);
    }
    if (ph > 3) {
        for (int dx = -ph + 3; dx <= ph - 3; ++dx) {
            Put(px + dx, bottom - 1, ((dx - frame / 4) % 7 == 0) ? kNeon : kNeonDark, 150);
        }
    }
    for (int sx : {bx - 10, bx + 9}) {
        for (int y = bottom - 4; y <= bottom; ++y) {
            const int half = 2 + (y - (bottom - 4)) / 2;
            for (int dx = -half; dx <= half; ++dx) {
                Put(sx + dx, y, (dx == -half || dx == half || y == bottom - 4) ? kWoodDark : kWood);
            }
        }
    }
    for (int y = by - 12; y <= by + 11; ++y) {
        const bool block = y <= by - 6 || y >= by + 5;
        for (int x = bx - 25; x <= bx - 22; ++x) {
            if (block || x >= bx - 23) {
                Put(x, y, (x == bx - 25 && block) ? kBomb : kBombDark);
            }
        }
    }
    for (int dx = 16; dx <= 21; ++dx) {
        const int hh = 6 - (dx - 16);
        for (int y = by - hh; y <= by + hh - 1; ++y) {
            Put(bx - dx, y, kBombDark);
        }
    }
    for (int j = 0; j < 20; ++j) {
        const double yy = (j - 9.5) / 10.0;
        const int half = static_cast<int>(std::lround(16.0 * std::sqrt(std::max(0.0, 1.0 - yy * yy))));
        const int y = by - 10 + j;
        for (int dx = -half; dx <= half; ++dx) {
            uint32_t c = j >= 16 ? kBombDark : kBomb;
            if (dx >= half - 1 && j > 2) {
                c = kBombDark;
            }
            if ((j == 2 || j == 3) && dx > -11 && dx < -3) {
                c = kBombLight;
            }
            if (j == 4 && dx > -9 && dx < -6) {
                c = kBombLight;
            }
            if ((dx == 9 || dx == 10) && j > 0 && j < 19) {
                c = ((j / 2) % 2 == 0) ? kHatYellow : kInk;
            }
            Put(bx + dx, y, c);
        }
    }
    const char* tre[11] = {"...YYYYY...", ".YYYYYYYYY.", ".YKKYYYKKY.", "YKKKKYKKKKY", "YKKKKYKKKKY", "YYYYYKYYYYY",
                           "YYYYYYYYYYY", "YYYYKKKYYYY", ".YYKKKKKYY.", ".YYYKKKYYY.", "...YYYYY..."};
    for (int j = 0; j < 11; ++j) {
        for (int i = 0; i < 11; ++i) {
            if (tre[j][i] != '.') {
                Put(bx - 8 + i, by - 6 + j, tre[j][i] == 'K' ? kInk : kHatYellow);
            }
        }
    }
    const int glow = 60 + 60 * ((frame / 4) % 2);
    for (int dx = -12; dx <= 12; dx += 3) {
        Put(bx + dx, by - 12 - ((dx / 3 + frame / 6) % 2 == 0 ? 1 : 0), kNeon, glow);
    }
    Put(bx + 12, by + 2, kNeon);
    Put(bx + 12, by + 3, kNeon);
    Put(px, by + 6, kNeon);
    Put(px, by + 7, kNeon);
    Put(px + 1, by + 7, kNeon);
    Put(px, by + 8, kNeon);
    const int fall = bottom - (by + 10);
    if (fall > 0) {
        Put(px, by + 10 + (frame / 2) % fall, kNeon);
        Put(px, by + 10 + (frame / 2 + fall / 2) % fall, kNeon, 200);
    }
}

void Canvas::DrawExplosion(int cx, int bottom, double t)
{
    if (t < 0.0) {
        return;
    }
    constexpr double kTau = 6.28318530717958;
    constexpr uint32_t kHot = 0xFFFDF0;
    constexpr uint32_t kPuff = 0xEDEAE6;
    constexpr uint32_t kPuffShade = 0xB9B2AC;
    constexpr uint32_t kBoomRed = 0xE8343B;
    const int cy = bottom - 20;

    if (t >= 0.45 && t < 2.5) {
        struct PuffSpot {
            double x, y, r, delay;
        };
        const PuffSpot spots[9] = {{0, -4, 11, 0.00}, {-12, 2, 8, 0.05}, {12, 1, 9, 0.08}, {-7, -14, 8, 0.12},
                                   {8, -15, 9, 0.10}, {0, -24, 8, 0.18}, {-18, 10, 6, 0.15}, {18, 11, 6, 0.20},
                                   {0, 12, 7, 0.02}};
        const double m = t - 0.45;
        double rad[9];
        int px[9];
        int py[9];
        for (int i = 0; i < 9; ++i) {
            const double g = std::clamp((m - spots[i].delay) / 0.3, 0.0, 1.0);
            const double shrink = std::clamp((m - 1.0 - spots[i].delay * 2.0) / 0.9, 0.0, 1.0);
            rad[i] = spots[i].r * (0.4 + 0.6 * g) * (1.0 - shrink) * (g > 0.0 ? 1.0 : 0.0);
            px[i] = cx + static_cast<int>(std::lround(spots[i].x * (0.8 + 0.3 * g)));
            py[i] = cy + static_cast<int>(std::lround(spots[i].y - m * 6.0));
        }
        for (int pass = 0; pass < 2; ++pass) {
            for (int i = 0; i < 9; ++i) {
                const double r = rad[i] + (pass == 0 ? 1.5 : 0.0);
                if (rad[i] < 1.0) {
                    continue;
                }
                const int ri = static_cast<int>(r) + 1;
                for (int dy = -ri; dy <= ri; ++dy) {
                    for (int dx = -ri; dx <= ri; ++dx) {
                        const double d = std::sqrt(static_cast<double>(dx * dx + dy * dy));
                        if (d > r || py[i] + dy > bottom) {
                            continue;
                        }
                        uint32_t c = kInk;
                        if (pass == 1) {
                            c = (dx + dy) > rad[i] * 0.55 ? kPuffShade : kPuff;
                            if (dx + dy < -rad[i] * 0.9 && d < rad[i] - 1.0) {
                                c = kHot;
                            }
                        }
                        Put(px[i] + dx, py[i] + dy, c);
                    }
                }
            }
        }
    }

    if (t < 1.1) {
        const double grow = std::min(1.0, t / 0.18);
        const double fade = t < 0.75 ? 1.0 : std::max(0.0, 1.0 - (t - 0.75) / 0.35);
        const double outer = 30.0 * grow * (0.35 + 0.65 * fade);
        const double spin = t * 1.5;
        auto starRadius = [&](double a) {
            const double f = std::fmod((a + spin) * 11.0 / kTau + 100.0, 1.0);
            const double tri = f < 0.5 ? f * 2.0 : (1.0 - f) * 2.0;
            const double wobble = 1.0 + 0.12 * std::sin(a * 5.0 + 1.3);
            return (0.55 + 0.45 * tri) * wobble;
        };
        const int ri = static_cast<int>(outer * 1.2) + 2;
        for (int dy = -ri; dy <= ri; ++dy) {
            for (int dx = -ri; dx <= ri; ++dx) {
                if (cy + dy > bottom) {
                    continue;
                }
                const double d = std::sqrt(static_cast<double>(dx * dx + dy * dy));
                const double k = starRadius(std::atan2(static_cast<double>(dy), static_cast<double>(dx)));
                const double edge = outer * k;
                if (d > edge + 1.3) {
                    continue;
                }
                uint32_t c = kInk;
                if (d <= edge) {
                    const double q = d / std::max(1.0, edge);
                    c = q < 0.3 ? kHot : (q < 0.55 ? kFlameYellow : (q < 0.8 ? kFlameOrange : kBoomRed));
                }
                Put(cx + dx, cy + dy, c);
            }
        }
        if (t >= 0.08) {
            const int bob = static_cast<int>(std::lround(std::sin(t * 18.0)));
            DrawBigText("BOOM!", cx, cy - 5 + bob, kFlameYellow, kInk);
        }
    }

    if (t < 0.12) {
        const int r = 6 + static_cast<int>(t * 160.0);
        const int a = static_cast<int>(255.0 * (1.0 - t / 0.12));
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                if (dx * dx + dy * dy <= r * r && cy + dy <= bottom) {
                    Put(cx + dx, cy + dy, kHot, a);
                }
            }
        }
    }

    if (t < 0.7) {
        const int r = 8 + static_cast<int>(t * 50.0);
        for (int s : {-1, 1}) {
            for (int k = 0; k < 4; k += 2) {
                const int x = cx + s * (r + k * 3);
                Put(x, bottom - 1 - k / 2, kInk);
                Put(x + s, bottom - 2 - k / 2, kInk);
                Put(x + 2 * s, bottom - 3 - k / 2, kInk);
            }
        }
    }
}

void Canvas::DrawAltar(int cx, int bottom, int frame, int glow)
{
    for (int dx = -6; dx <= 6; ++dx) {
        Put(cx + dx, bottom - 4, kStoneLight);
    }
    for (int y = bottom - 3; y <= bottom; ++y) {
        for (int dx = -5; dx <= 5; ++dx) {
            Put(cx + dx, y, (dx + y) % 5 == 0 ? kStoneLight : kStone);
        }
    }
    for (int y = bottom - 3; y <= bottom - 1; ++y) {
        for (int dx = -2; dx <= 2; ++dx) {
            Put(cx + dx, y, y == bottom - 1 ? kRobeTrim : kRobe);
        }
    }
    for (int side : {-5, 5}) {
        for (int y = bottom - 7; y <= bottom - 5; ++y) {
            Put(cx + side, y, kCandle);
        }
        const bool f = ((frame + (side > 0 ? 2 : 0)) / 3) % 2 == 0;
        Put(cx + side, bottom - 8, kFlameYellow);
        Put(cx + side + (f ? 0 : 1), bottom - 9, f ? kFlameOrange : kFlameYellow, 200);
    }
    const int bobs[4] = {0, 1, 2, 1};
    const int sy = bottom - 21 + bobs[(frame / 6) % 4];
    const int a = std::clamp(glow, 0, 255);
    for (int k = 0; k < 14; ++k) {
        const double ang = 2.0 * 3.14159265358979 * k / 14.0 + frame * 0.05;
        const int x = cx + static_cast<int>(std::lround(8.0 * std::cos(ang)));
        const int y = sy + 2 + static_cast<int>(std::lround(5.0 * std::sin(ang)));
        Put(x, y, kGold, a / 2);
    }
    DrawGlyph('{', cx - 4, sy, kGold, std::max(140, a));
    DrawGlyph('}', cx + 2, sy, kGold, std::max(140, a));
}

HICON CreateCrabIcon(int size)
{
    Canvas c;
    c.Resize(16, 16, std::max(1, size / 16));
    c.DrawCrab(Pose{}, 0, 3);
    const int w = c.PixelWidth();
    const int h = c.PixelHeight();

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HDC screen = GetDC(nullptr);
    HBITMAP color = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, screen);
    if (!color || !bits) {
        return nullptr;
    }
    std::memcpy(bits, c.Data(), static_cast<size_t>(w) * h * 4);

    std::vector<BYTE> zeros(static_cast<size_t>((w + 15) / 16 * 2) * h, 0);
    HBITMAP mask = CreateBitmap(w, h, 1, 1, zeros.data());
    ICONINFO ii{};
    ii.fIcon = TRUE;
    ii.hbmMask = mask;
    ii.hbmColor = color;
    HICON icon = CreateIconIndirect(&ii);
    DeleteObject(color);
    DeleteObject(mask);
    return icon;
}

void Canvas::DrawSignBubble(int cx, int bottom, int sign)
{
    const int w = 11;
    const int x0 = std::clamp(cx - w / 2, 0, std::max(0, lw_ - w));
    const int y0 = bottom - 12;
    for (int j = 0; j < 10; ++j) {
        for (int i = 0; i < w; ++i) {
            const bool edgeX = i == 0 || i == w - 1;
            const bool edgeY = j == 0 || j == 9;
            if (edgeX && edgeY) {
                continue;
            }
            Put(x0 + i, y0 + j, (edgeX || edgeY) ? kInk : kPaper);
        }
    }
    const int tx = std::clamp(cx, x0 + 2, x0 + w - 3);
    Put(tx, y0 + 9, kPaper);
    Put(tx, y0 + 10, kInk);
    static const char* icons[3][6] = {
        {".....", ".RRR.", "RLRRR", "RRRRR", ".RRR.", "....."},
        {"KKKK.", "KWWKK", "KWWWK", "KBBWK", "KWWWK", "KKKKK"},
        {"M...M", ".M.M.", "..M..", "CC.CC", "CC.CC", "....."},
    };
    const int k = std::clamp(sign, 0, 2);
    for (int j = 0; j < 6; ++j) {
        for (int i = 0; i < 5; ++i) {
            uint32_t c = 0;
            switch (icons[k][j][i]) {
            case 'R': c = kStone; break;
            case 'L': c = kStoneLight; break;
            case 'K': c = kInk; break;
            case 'W': c = kBlank; break;
            case 'B': c = kKeyB; break;
            case 'M': c = kMetal; break;
            case 'C': c = kChuteRed; break;
            default: break;
            }
            if (c) {
                Put(x0 + 3 + i, y0 + 2 + j, c);
            }
        }
    }
}

void Canvas::DrawRope(int x0, int x1, int y, int sag)
{
    if (x1 < x0) {
        std::swap(x0, x1);
    }
    const double half = std::max(1.0, (x1 - x0) / 2.0);
    const double mid = (x0 + x1) / 2.0;
    for (int x = x0; x <= x1; ++x) {
        const double q = (x - mid) / half;
        const int yy = y + static_cast<int>(std::lround(sag * (1.0 - q * q)));
        Put(x, yy, (x / 2) % 2 ? kStraw : kStrawDark);
    }
    const int mx = static_cast<int>(std::lround(mid));
    const int my = y + sag;
    Put(mx, my - 1, kChuteRed);
    Put(mx, my + 1, kChuteRed);
    Put(mx + 1, my + 1, kChuteRed);
    Put(mx + 1, my + 2, kChuteRed);
}

void Canvas::DrawPongTable(int cx, int bottom)
{
    constexpr uint32_t kTable = 0x2F6B4F;
    constexpr uint32_t kTableDark = 0x1F4A36;
    const int top = bottom - 7;
    for (int x : {cx - 11, cx + 11}) {
        for (int y = top + 2; y <= bottom; ++y) {
            Put(x, y, kPanEdge);
        }
    }
    for (int dx = -13; dx <= 13; ++dx) {
        Put(cx + dx, top, (dx == -13 || dx == 13) ? kPaper : (dx == 0 ? kPaper : kTable));
        Put(cx + dx, top + 1, kTableDark);
    }
    for (int y = top - 3; y <= top - 1; ++y) {
        Put(cx, y, y == top - 3 ? kPaper : kToqueShade);
    }
    Put(cx - 1, top - 3, kPaper);
    Put(cx + 1, top - 3, kPaper);
}

void Canvas::DrawPicnic(int cx, int bottom, int antX, bool antCarry, int crumbX)
{
    for (int y = bottom - 1; y <= bottom; ++y) {
        for (int dx = -20; dx <= 20; ++dx) {
            const bool red = ((dx + 40) / 3 + (y == bottom ? 1 : 0)) % 2 == 0;
            Put(cx + dx, y, red ? kChuteRed : kPaper);
        }
    }
    for (int y = bottom - 6; y <= bottom - 2; ++y) {
        for (int dx = -4; dx <= 4; ++dx) {
            Put(cx + dx, y, (dx + y) % 2 ? kWood : kWoodDark);
        }
    }
    for (int dx = -5; dx <= 5; ++dx) {
        Put(cx + dx, bottom - 7, kWoodDark);
    }
    const int handle[7][2] = {{-4, 8}, {-4, 9}, {-3, 10}, {3, 10}, {4, 9}, {4, 8}, {0, 11}};
    for (const auto& hp : handle) {
        Put(cx + hp[0], bottom - hp[1], kWoodDark);
    }
    for (int dx = -2; dx <= 2; ++dx) {
        Put(cx + dx, bottom - 11, kWoodDark);
    }
    Put(cx + 2, bottom - 8, kChuteRed);
    Put(cx + 3, bottom - 8, kPaper);
    Put(cx - 2, bottom - 8, kStraw);
    if (crumbX != -999) {
        Put(crumbX, bottom - 2, kStraw);
    }
    if (antX != -999) {
        Put(antX, bottom - 2, kInk);
        Put(antX + 1, bottom - 2, kInk);
        Put(antX + 2, bottom - 3, kInk);
        Put(antX, bottom - 1, kInk);
        Put(antX + 2, bottom - 1, kInk);
        if (antCarry) {
            Put(antX + 1, bottom - 4, kStraw);
            Put(antX + 2, bottom - 4, kStraw);
        }
    }
}

void Canvas::DrawChess(int cx, int bottom, int moves, bool flipped)
{
    for (int dx = -8; dx <= 8; ++dx) {
        Put(cx + dx, bottom - 6, kWood);
    }
    for (int y = bottom - 5; y <= bottom; ++y) {
        Put(cx - 6, y, kWoodDark);
        Put(cx + 6, y, kWoodDark);
    }
    if (flipped) {
        return;
    }
    for (int y = bottom - 8; y <= bottom - 7; ++y) {
        for (int dx = -6; dx <= 5; ++dx) {
            Put(cx + dx, y, ((dx + y) % 2 == 0) ? kBlank : kTopHatLight);
        }
    }
    const int white[4] = {-6, -5, -4, -3};
    const int black[4] = {5, 4, 3, 2};
    for (int i = 0; i < 4; ++i) {
        const int wx = white[i] + (i < (moves + 1) / 2 ? 3 : 0);
        const int bx = black[i] - (i < moves / 2 ? 3 : 0);
        Put(cx + wx, bottom - 9, kPaper);
        Put(cx + bx, bottom - 9, kInk);
        if (i == 0) {
            Put(cx + wx, bottom - 10, kPaper);
            Put(cx + wx, bottom - 11, kGold);
            Put(cx + bx, bottom - 10, kInk);
            Put(cx + bx, bottom - 11, kGold);
        }
    }
}

void Canvas::DrawSeesaw(int cx, int bottom, int half, int tiltRight)
{
    for (int j = 0; j <= 4; ++j) {
        for (int dx = -j; dx <= j; ++dx) {
            Put(cx + dx, bottom - 4 + j, dx == j ? kStone : kStoneLight);
        }
    }
    for (int dx = -half; dx <= half; ++dx) {
        const int yy = bottom - 6 - static_cast<int>(std::lround(static_cast<double>(tiltRight) * dx / half));
        Put(cx + dx, yy, kWood);
        Put(cx + dx, yy + 1, kWoodDark);
    }
    for (int s : {-1, 1}) {
        const int dx = s * (half - 3);
        const int yy = bottom - 6 - static_cast<int>(std::lround(static_cast<double>(tiltRight) * dx / half));
        Put(cx + dx, yy - 1, kWoodDark);
        Put(cx + dx, yy - 2, kWoodDark);
        Put(cx + dx + s, yy - 2, kWoodDark);
    }
}

void Canvas::DrawDeploy(int cx, int bottom, bool pressed, int alarm)
{
    for (int y = bottom - 8; y <= bottom; ++y) {
        for (int dx = -3; dx <= 3; ++dx) {
            Put(cx + dx, y, dx == -3 ? kMetal : (dx == 3 ? kPanIn : kPanEdge));
        }
    }
    for (int dx = -4; dx <= 4; ++dx) {
        Put(cx + dx, bottom - 9, kPanIn);
    }
    if (pressed) {
        for (int dx = -2; dx <= 2; ++dx) {
            Put(cx + dx, bottom - 10, kFlameRed);
        }
    } else {
        for (int dx = -2; dx <= 2; ++dx) {
            Put(cx + dx, bottom - 10, kFlameRed);
        }
        for (int dx = -1; dx <= 1; ++dx) {
            Put(cx + dx, bottom - 11, dx < 0 ? kFlameOrange : kChuteRed);
        }
    }
    const int top = bottom - 27;
    for (int x : {cx - 13, cx + 13}) {
        for (int y = top + 8; y <= bottom; ++y) {
            Put(x, y, kWoodDark);
        }
    }
    const bool flash = alarm > 0 && (alarm / 3) % 2 == 0;
    for (int y = top; y <= top + 8; ++y) {
        for (int dx = -14; dx <= 14; ++dx) {
            const bool edge = y == top || y == top + 8 || dx == -14 || dx == 14;
            Put(cx + dx, y, edge ? kInk : (flash ? kHatYellowLight : kBlank));
        }
    }
    DrawText("DEPLOY", cx - TextWidth("DEPLOY") / 2, top + 2, flash ? kInk : kFlameRed);
    if (alarm > 0) {
        const uint32_t c = (alarm / 2) % 2 ? kFlameRed : kFlameOrange;
        for (int dx = -1; dx <= 1; ++dx) {
            Put(cx + dx, top - 1, kPanEdge);
        }
        Put(cx, top - 2, c);
        Put(cx, top - 3, c);
        Put(cx - 1, top - 2, c);
        Put(cx + 1, top - 2, c);
        const int k = alarm % 8;
        Put(cx + kRing[k][0] * 3, top - 2 + kRing[k][1] * 2, c, 160);
        Put(cx - kRing[k][0] * 3, top - 2 - kRing[k][1] * 2, c, 160);
    }
}

void Canvas::DrawBoard(int cx, int bottom, int notes)
{
    for (int x : {cx - 11, cx + 11}) {
        for (int y = bottom - 14; y <= bottom; ++y) {
            Put(x, y, kPanEdge);
        }
    }
    const int top = bottom - 27;
    for (int y = top; y <= bottom - 14; ++y) {
        for (int dx = -14; dx <= 14; ++dx) {
            const bool edge = y == top || y == bottom - 14 || dx == -14 || dx == 14;
            Put(cx + dx, y, edge ? kPanEdge : kBlank);
        }
    }
    for (int dx = -12; dx <= -6; ++dx) {
        Put(cx + dx, top + 2, kKeyB);
    }
    const int spots[6][2] = {{-11, 4}, {-6, 4}, {-1, 4}, {4, 4}, {-9, 8}, {-4, 8}};
    const uint32_t colors[3] = {kHatYellow, kPetal, kPorthole};
    for (int i = 0; i < std::min(notes, 6); ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                Put(cx + spots[i][0] + k, top + spots[i][1] + j, colors[i % 3]);
            }
        }
    }
    for (int x = cx + 9; x <= cx + 12; ++x) {
        Put(x, top + 10, kChuteRed);
    }
    Put(cx + 12, top + 9, kChuteRed);
    Put(cx + 12, top + 11, kChuteRed);
}

void Canvas::DrawDrums(int cx, int bottom, int hit)
{
    const int rows[7] = {2, 4, 5, 5, 5, 4, 2};
    for (int j = 0; j < 7; ++j) {
        for (int dx = -rows[j]; dx <= rows[j]; ++dx) {
            const bool rim = std::abs(dx) == rows[j] || j == 0 || j == 6;
            Put(cx + dx, bottom - 6 + j, rim ? kChuteRed : kBlank);
        }
    }
    Put(cx - 1, bottom - 3, kPanEdge);
    Put(cx, bottom - 3, kPanEdge);
    Put(cx + 1, bottom - 3, kPanEdge);
    for (int dx = 5; dx <= 9; ++dx) {
        Put(cx + dx, bottom - 5 + (hit == 2 ? 1 : 0), kMetal);
        Put(cx + dx, bottom - 4 + (hit == 2 ? 1 : 0), kPanEdge);
    }
    for (int y = bottom - 3; y <= bottom; ++y) {
        Put(cx + 7, y, kPanEdge);
    }
    const int tilt = hit == 1 ? 1 : 0;
    for (int dx = -11; dx <= -5; ++dx) {
        Put(cx + dx, bottom - 11 + (dx < -8 ? tilt : -tilt), kGold);
    }
    for (int y = bottom - 10; y <= bottom; ++y) {
        Put(cx - 8, y, kPanEdge);
    }
}

void Canvas::DrawDemon(int cx, int y, int alpha)
{
    const int a = std::clamp(alpha, 0, 255);
    if (a <= 0) {
        return;
    }
    DrawGlyph('{', cx - 6, y, kFlameRed, a);
    DrawGlyph('}', cx + 4, y, kFlameRed, a);
    Put(cx - 2, y + 2, kHatYellowLight, a);
    Put(cx + 2, y + 2, kHatYellowLight, a);
    Put(cx - 1, y + 4, kFlameRed, a);
    Put(cx, y + 4, kFlameRed, a);
    Put(cx + 1, y + 4, kFlameRed, a);
    Put(cx - 5, y - 1, kFlameRed, a);
    Put(cx + 5, y - 1, kFlameRed, a);
    Put(cx - 6, y - 2, kFlameRed, a);
    Put(cx + 6, y - 2, kFlameRed, a);
}
