#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>

enum class Eyes { Open, Closed, Happy, Surprised };
enum class Claw { High, Up, Mid, Down };
enum class Legs { Stand, LiftLeft, LiftRight, Spread };
enum class Body { Normal, Crouch, Squash };

/** Drawing parameters of one crab frame. */
struct Pose {
    Eyes eyes = Eyes::Open;
    int lookX = 0;
    int lookY = 0;
    Claw left = Claw::Mid;
    Claw right = Claw::Mid;
    Legs legs = Legs::Stand;
    Body body = Body::Normal;
    bool keyboard = false;
    int pressedKey = -1;
};

constexpr int kCrabW = 16;
constexpr int kCrabH = 10;

/** Premultiplied BGRA buffer drawn in logical pixels of scale x scale screen pixels. */
class Canvas {
public:
    void Resize(int logicalW, int logicalH, int scale);
    void Clear();
    void Put(int x, int y, uint32_t rgb, int alpha = 255);
    void DrawCrab(const Pose& pose, int x, int y);
    void DrawGlyph(char c, int x, int y, uint32_t rgb, int alpha);
    void DrawBubble(int x, int y, int dots);
    int PixelWidth() const { return lw_ * scale_; }
    int PixelHeight() const { return lh_ * scale_; }
    const uint32_t* Data() const { return px_.data(); }

private:
    int lw_ = 0;
    int lh_ = 0;
    int scale_ = 1;
    std::vector<uint32_t> px_;
};

/** Builds a square icon of the crab, size in pixels. */
HICON CreateCrabIcon(int size);
