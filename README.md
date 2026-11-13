# ClaudePoo

A desktop pet for Windows, in the spirit of the old Screen Mate Poo: the small orange pixel
crab of Claude appears at random from behind one of your windows, sits on its title bar,
types on a tiny keyboard and does small cute things, then hides again.

Pure Win32 + GDI, C++20, no external library. One small exe, no install.

## Build

Needs Visual Studio 2026 (or Build Tools 2026) with the C++ x64 tools, and CMake.

```
build.cmd
```

Output: `build\claudepoo.exe`. `build.cmd` finds MSVC with vswhere. If vcvars does not find
the Windows SDK (missing registry entry), it takes the newest SDK found under
`C:\Program Files (x86)\Windows Kits\10`.

## Run

Start `build\claudepoo.exe`. The crab shows up after 3 s, then at random times.
Starting the exe a second time makes the running crab appear now.

Tray icon (right click):

- Show now / Hide
- Play: start an animation now (Peek, Parachute, DIY on a ladder, Paint, Magic, Cook, Dizzy)
- Frequency: Rare (5-15 min), Normal (1-5 min), Often (20-60 s)
- Pause
- Start with Windows (`HKCU\Software\Microsoft\Windows\CurrentVersion\Run\ClaudePoo`)
- Quit

Double click on the tray icon = Show now. Right click on the crab opens the same menu.
Settings are stored in `HKCU\Software\ClaudePoo`.

## What the crab does

- Peeks from behind the top edge of a window (or from behind its left/right side), looks
  around, sometimes gets shy and goes back, otherwise climbs out with a hop.
- Then 4 to 8 activities: typing on a keyboard (code glyphs pop up), walking along the edge,
  waving at the mouse, following the mouse with its eyes, dancing, sleeping (wakes up when
  the mouse comes close), thinking, sneezing.
- Short special animations, picked at random among the activities:
  - DIY: puts on a hard hat, climbs a small ladder and hammers; the ladder vanishes and he
    falls down, dizzy for a moment.
  - Painter: beret and brush; he looks toward the wallpaper and paints a small copy of your
    real wallpaper on an easel.
  - Magician: top hat and wand; shooting stars, then a bow.
  - Cook: chef toque and a frying pan; vegetables jump in the pan, steam rises.
- Sometimes, when the cursor comes near, he stops and stares at it. Not every time: 30 %
  chance, then a pause of 15-30 s. If you circle the cursor around him while he stares, he
  gets dizzy.
- Sometimes (15 % of the visits) he arrives by parachute from the top of a screen down to
  the taskbar.
- If another window covers him for 2 s, he leaves and comes back elsewhere a bit later.
- Waves goodbye and slides back behind the window.
- If no window edge is free (for example a maximized window), it uses the top of the taskbar.
- Follows the window when you move it. Falls when the window is minimized or closed.
- Click: hop with hearts, happy wiggle, or it runs away.
- Drag and drop: grab it, throw it; it lands on the first window top edge below, or on the
  taskbar.

## How it works

- `src/Mate.cpp`: layered window (`UpdateLayeredWindow`, per-pixel alpha), the state machine
  and the activities. To "come from behind" a window, the crab window is put just below the
  target window in the z-order (`SetWindowPos(crab, target)`), then slides up: Windows draws
  the real window over the hidden part. Once out, it is kept just above the target.
- `src/Canvas.cpp`: the crab is drawn in code, pixel by pixel, from a `Pose` (eyes, claws,
  legs, body). Scale 4 at 100 % DPI, per-monitor DPI aware.
- `src/Scanner.cpp`: `EnumWindows` + DWM frame bounds, in z-order, to find visible edges.
- `src/Tray.cpp`, `src/Settings.cpp`: tray icon, menu, registry.
