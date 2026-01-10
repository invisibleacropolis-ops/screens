#pragma once

#include "../Config.h"
#include "../resource.h"
#include <commctrl.h>
#include <windows.h>


// Interface for modular settings panels
// Each panel handles a group of related settings controls
class ISettingsPanel {
public:
  virtual ~ISettingsPanel() = default;

  // Get the display name of this panel (e.g., "Visual Effects")
  virtual const wchar_t *GetName() const = 0;

  // Add this panel's controls to the dialog template
  // y is the current Y position; update it after adding controls
  // Returns the number of dialog units added (height)
  virtual int AddControls(LPWORD &p, int startY, int dialogWidth) = 0;

  // Initialize controls from config when dialog opens
  virtual void InitFromConfig(HWND hDlg, const Config &cfg) = 0;

  // Read control values back to config
  virtual void ReadToConfig(HWND hDlg, Config &cfg) = 0;

  // Handle WM_HSCROLL for sliders in this panel
  // Return true if handled
  virtual bool HandleSlider(HWND hDlg, int sliderId) = 0;
};

// Utility functions for panels
namespace SettingsUtils {

inline void SetSliderValue(HWND hDlg, int sliderId, int labelId, int value,
                           int minVal, int maxVal) {
  HWND hSlider = GetDlgItem(hDlg, sliderId);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
  SendMessage(hSlider, TBM_SETPOS, TRUE, value);

  wchar_t buf[32];
  swprintf_s(buf, L"%d", value);
  SetDlgItemTextW(hDlg, labelId, buf);
}

inline void SetSliderValueFloat(HWND hDlg, int sliderId, int labelId,
                                float value, float minVal, float maxVal,
                                int steps = 100) {
  HWND hSlider = GetDlgItem(hDlg, sliderId);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, steps));
  int pos = (int)((value - minVal) / (maxVal - minVal) * steps);
  SendMessage(hSlider, TBM_SETPOS, TRUE, pos);

  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", value);
  SetDlgItemTextW(hDlg, labelId, buf);
}

inline int GetSliderValue(HWND hDlg, int sliderId) {
  return (int)SendMessage(GetDlgItem(hDlg, sliderId), TBM_GETPOS, 0, 0);
}

inline float GetSliderValueFloat(HWND hDlg, int sliderId, float minVal,
                                 float maxVal, int steps = 100) {
  int pos = GetSliderValue(hDlg, sliderId);
  return minVal + (float)pos / steps * (maxVal - minVal);
}

inline void UpdateSliderLabel(HWND hDlg, int sliderId, int labelId,
                              float minVal, float maxVal, int steps = 100) {
  float value = GetSliderValueFloat(hDlg, sliderId, minVal, maxVal, steps);
  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", value);
  SetDlgItemTextW(hDlg, labelId, buf);
}

inline void InitMeshCombo(HWND hDlg, int comboId, MeshType selected) {
  HWND hCombo = GetDlgItem(hDlg, comboId);
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Sphere");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Cube");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Ring");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"None");
  SendMessage(hCombo, CB_SETCURSEL, (int)selected, 0);
}

} // namespace SettingsUtils
