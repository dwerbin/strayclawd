#pragma once

#include <windows.h>

#include <functional>

#include "Settings.h"

/** Shows the animation settings dialog; test plays one animation now. Returns true and updates the weights on OK. */
bool RunConfigDialog(HINSTANCE instance, Settings& settings, const std::function<void(int)>& test);
