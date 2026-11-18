#pragma once

#include <windows.h>

#include "Settings.h"

/** Shows the animation settings dialog. Returns true and updates the animation weights on OK. */
bool RunConfigDialog(HINSTANCE instance, Settings& settings);
