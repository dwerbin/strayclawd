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

    const int ey = top + 2 + p.lookY;
    const int eyeCols[2] = {4 + p.lookX, 11 + p.lookX};
    for (int i = 0; i < 2; ++i) {
        const int ex = eyeCols[i];
        switch (p.eyes) {
        case Eyes::Open:
            Put(ox + ex, oy + ey, kInk);
            Put(ox + ex, oy + ey + 1, kInk);
            break;
        case Eyes::Closed:
            Put(ox + ex - (i == 0 ? 1 : 0), oy + ey + 1, kInk);
            Put(ox + ex + (i == 0 ? 0 : 1), oy + ey + 1, kInk);
            break;
        case Eyes::Happy:
            Put(ox + ex, oy + ey, kInk);
            Put(ox + ex - 1, oy + ey + 1, kInk);
            Put(ox + ex + 1, oy + ey + 1, kInk);
            break;
        case Eyes::Surprised: {
            const int c0 = i == 0 ? ex - 1 : ex;
            for (int y = ey - 1; y <= ey + 1; ++y) {
                Put(ox + c0, oy + y, kInk);
                Put(ox + c0 + 1, oy + y, kInk);
            }
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
                Put(ox + cx + kRing[k][0], oy + cy + kRing[k][1], kInk);
            }
            break;
        }
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
        const int widths[3] = {7, 9, 11};
        for (int j = 0; j < 3; ++j) {
            for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
                if (j == 2 && (dx + 11) % 4 == 3) {
                    continue;
                }
                const uint32_t c = ((dx + 22) / 3) % 2 ? kUmbrella : kPaper;
                tp(14 + dx, top - 7 + j, c);
            }
        }
        tp(14, top - 8, kInk);
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
