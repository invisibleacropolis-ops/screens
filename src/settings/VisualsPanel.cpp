#include "VisualsPanel.h"

#include "../resource.h"
#include <commctrl.h>

namespace {

void SetSliderValue(HWND hDlg, int sliderId, int labelId, int value, int minVal,
                    int maxVal) {
  HWND hSlider = GetDlgItem(hDlg, sliderId);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
  SendMessage(hSlider, TBM_SETPOS, TRUE, value);
  ShowWindow(hSlider, SW_SHOW);

  wchar_t buf[32];
  swprintf_s(buf, L"%d", value);
  SetDlgItemTextW(hDlg, labelId, buf);
}

void SetSliderValueFloat(HWND hDlg, int sliderId, int labelId, float value,
                         float minVal, float maxVal, int steps = 100) {
  HWND hSlider = GetDlgItem(hDlg, sliderId);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, steps));
  int pos = (int)((value - minVal) / (maxVal - minVal) * steps);
  SendMessage(hSlider, TBM_SETPOS, TRUE, pos);
  ShowWindow(hSlider, SW_SHOW);

  wchar_t buf[32];
  swprintf_s(buf, L"%.2f", value);
  SetDlgItemTextW(hDlg, labelId, buf);
}

int GetSliderValue(HWND hDlg, int sliderId) {
  return (int)SendMessage(GetDlgItem(hDlg, sliderId), TBM_GETPOS, 0, 0);
}

float GetSliderValueFloat(HWND hDlg, int sliderId, float minVal, float maxVal,
                          int steps = 100) {
  int pos = GetSliderValue(hDlg, sliderId);
  return minVal + (float)pos / steps * (maxVal - minVal);
}

void UpdateSliderLabel(HWND hDlg, int sliderId, int labelId, bool isFloat,
                       float minVal = 0, float maxVal = 100, int steps = 100) {
  int pos = GetSliderValue(hDlg, sliderId);
  wchar_t buf[32];
  if (isFloat) {
    float value = minVal + (float)pos / steps * (maxVal - minVal);
    swprintf_s(buf, L"%.2f", value);
  } else {
    swprintf_s(buf, L"%d", pos);
  }
  SetDlgItemTextW(hDlg, labelId, buf);
}

} // namespace

namespace SettingsPanels {

void InitVisualsTab(HWND hDlg, const Config &cfg) {
  CheckDlgButton(hDlg, IDC_CHECK_BLOOM,
                 cfg.bloomEnabled ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_CHECK_FXAA,
                 cfg.fxaaEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_THRESH, IDC_STATIC_BLOOM_THRESH,
                      cfg.bloomThreshold, 0.3f, 1.5f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_STR, IDC_STATIC_BLOOM_STR,
                      cfg.bloomStrength, 0.0f, 2.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_EXPOSURE, IDC_STATIC_EXPOSURE,
                      cfg.exposure, 0.5f, 3.0f);
  CheckDlgButton(hDlg, IDC_CHECK_FOG,
                 cfg.fogEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FOG_DENSITY, IDC_STATIC_FOG_DENSITY,
                      cfg.fogDensity, 0.0f, 0.2f);
  CheckDlgButton(hDlg, IDC_CHECK_PARTICLES,
                 cfg.particlesEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValue(hDlg, IDC_SLIDER_PARTICLE_CNT, IDC_STATIC_PARTICLE_CNT,
                 cfg.particleCount, 500, 10000);
}

void ReadVisualsTab(HWND hDlg, Config &cfg) {
  cfg.bloomEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_BLOOM) == BST_CHECKED;
  cfg.fxaaEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_FXAA) == BST_CHECKED;
  cfg.bloomThreshold =
      GetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_THRESH, 0.3f, 1.5f);
  cfg.bloomStrength =
      GetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_STR, 0.0f, 2.0f);
  cfg.exposure = GetSliderValueFloat(hDlg, IDC_SLIDER_EXPOSURE, 0.5f, 3.0f);
  cfg.fogEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_FOG) == BST_CHECKED;
  cfg.fogDensity =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FOG_DENSITY, 0.0f, 0.2f);
  cfg.particlesEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_PARTICLES) == BST_CHECKED;
  cfg.particleCount = GetSliderValue(hDlg, IDC_SLIDER_PARTICLE_CNT);
}

bool HandleVisualSliders(HWND hDlg, int sliderId) {
  switch (sliderId) {
  case IDC_SLIDER_BLOOM_THRESH:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BLOOM_THRESH, true, 0.3f,
                      1.5f);
    return true;
  case IDC_SLIDER_BLOOM_STR:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BLOOM_STR, true, 0.0f, 2.0f);
    return true;
  case IDC_SLIDER_EXPOSURE:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_EXPOSURE, true, 0.5f, 3.0f);
    return true;
  case IDC_SLIDER_FOG_DENSITY:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FOG_DENSITY, true, 0.0f, 0.2f);
    return true;
  case IDC_SLIDER_PARTICLE_CNT:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_PARTICLE_CNT, false, 500,
                      10000);
    return true;
  default:
    return false;
  }
}

} // namespace SettingsPanels
