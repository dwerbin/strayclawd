#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>

enum class Eyes { Open, Closed, Happy, Surprised, Dizzy };
enum class Claw { High, Up, Mid, Down };
enum class Legs { Stand, LiftLeft, LiftRight, Spread };
enum class Body { Normal, Crouch, Squash };
enum class Hat { None, HardHat, Beret, TopHat, Toque, Nightcap, Santa, Witch, Party, Wizard };
enum class Tool { None, Hammer, Brush, Wand, Pan, Rod, Mug, Can, Umbrella, Staff, Duster, Broom };

constexpr int kNoBarbell = -99;

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
    Hat hat = Hat::None;
    int hatDrop = 0;
    Tool tool = Tool::None;
    int toolSide = 1;
    int toolLift = 0;
    uint32_t brushColor = 0xE8505B;
    int frame = 0;
    bool mouth = false;
    bool glasses = false;
    int book = -1;
    int barbell = kNoBarbell;
    bool skate = false;
    bool roundGlasses = false;
    bool scar = false;
    bool scarf = false;
    bool beard = false;
    bool broom = false;
    bool woodWand = false;
    int glow = 0;
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
    void DrawLadder(int x, int bottom, int height);
    void DrawEasel(int x, int y, const uint32_t* painting, const bool* revealed);
    void DrawParachute(int cx, int top, int fold);
    void DrawPlant(int x, int bottom, int level);
    void DrawRainbow(int cx, int cy, int alpha);
    void DrawRocket(int x, int y, int level, int flame);
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t rgb, int alpha = 255);
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
