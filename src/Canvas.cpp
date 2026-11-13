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
};

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
        row(hb - 3, 5, 10, kHatYellow);
        row(hb - 2, 4, 11, kHatYellow);
        row(hb - 1, 3, 12, kHatYellowDark);
        Put(ox + 6, oy + hb - 3, kHatYellowLight);
        Put(ox + 5, oy + hb - 2, kHatYellowLight);
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
    case Hat::None:
        break;
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
    auto tp = [&](int x, int y, uint32_t c) { Put(ox + (side > 0 ? x : 15 - x), oy + y, c); };
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
            tp(16, y, kTopHat);
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
    case Tool::None:
        break;
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
        const int widths[5] = {5, 7, 8, 9, 9};
        for (int j = 0; j < 5; ++j) {
            for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
                if (j == 4 && (dx + 9) % 6 == 3) {
                    continue;
                }
                Put(cx + dx, top + j, stripe(dx));
            }
        }
        const int from[4] = {-9, -3, 3, 9};
        const int to[4] = {-8, -4, 3, 7};
        for (int k = 0; k < 4; ++k) {
            for (int y = top + 5; y <= top + 13; ++y) {
                const double t = (y - (top + 5)) / 8.0;
                const int x = cx + static_cast<int>(std::lround(from[k] + (to[k] - from[k]) * t));
                Put(x, y, kString, 220);
            }
        }
    } else if (fold == 1) {
        const int widths[3] = {4, 6, 7};
        for (int j = 0; j < 3; ++j) {
            for (int dx = -widths[j]; dx <= widths[j]; ++dx) {
                Put(cx + dx, top + 7 + j, stripe(dx));
            }
        }
    } else {
        for (int dx = -10; dx <= 10; ++dx) {
            Put(cx + dx, top + 22, stripe(dx));
        }
        for (int dx = -11; dx <= 11; ++dx) {
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
