#include "Canvas.h"

#include <algorithm>
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
};

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

    const int lr = top + ClawRow(p.left);
    const int rr = top + ClawRow(p.right);
    for (int d = 0; d < 2; ++d) {
        Put(ox + 0, oy + lr + d, kOrange);
        Put(ox + 1, oy + lr + d, kOrange);
        Put(ox + 14, oy + rr + d, kOrange);
        Put(ox + 15, oy + rr + d, kOrange);
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
