#pragma once

#include "../Config.h"
#include <windows.h>

namespace SettingsPanels {

void InitMetricsTab(HWND hDlg, const Config &cfg);
void ReadMetricsTab(HWND hDlg, Config &cfg);
bool HandleMetricsSliders(HWND hDlg, int sliderId);

} // namespace SettingsPanels
