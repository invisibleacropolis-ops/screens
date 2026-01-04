// Settings Dialog Interface
#pragma once

#include "Config.h"
#include <windows.h>


// Shows the settings dialog and returns the updated config
// Returns true if user clicked OK, false if cancelled
bool ShowSettingsDialog(HINSTANCE hInstance, HWND parent, Config &config);
