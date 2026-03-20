#pragma once

#include "../Config.h"
#include <windows.h>

namespace SettingsPanels {

void InitVisualsTab(HWND hDlg, const Config &cfg);
void ReadVisualsTab(HWND hDlg, Config &cfg);
bool HandleVisualSliders(HWND hDlg, int sliderId);

} // namespace SettingsPanels
