#pragma once

#include "../Config.h"
#include <windows.h>

namespace SettingsPanels {

void InitGeneralTab(HWND hDlg, const Config &cfg);
void ReadGeneralTab(HWND hDlg, Config &cfg);
bool HandleGeneralSliders(HWND hDlg, int sliderId);

} // namespace SettingsPanels
