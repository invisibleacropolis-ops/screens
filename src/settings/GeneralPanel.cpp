#include "GeneralPanel.h"

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

void InitGeneralTab(HWND hDlg, const Config &cfg) {
  HWND hQuality = GetDlgItem(hDlg, IDC_COMBO_QUALITY);
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"Low");
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"Medium");
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"High");
  SendMessage(hQuality, CB_SETCURSEL, (int)cfg.quality, 0);

  SetSliderValueFloat(hDlg, IDC_SLIDER_ROTATION, IDC_STATIC_ROTATION,
                      cfg.rotationSpeed, 0.0f, 2.0f);
  CheckDlgButton(hDlg, IDC_CHECK_SKYBOX,
                 cfg.skyboxEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValue(hDlg, IDC_SLIDER_BG_R, IDC_STATIC_BG_R, cfg.bgColorR, 0, 255);
  SetSliderValue(hDlg, IDC_SLIDER_BG_G, IDC_STATIC_BG_G, cfg.bgColorG, 0, 255);
  SetSliderValue(hDlg, IDC_SLIDER_BG_B, IDC_STATIC_BG_B, cfg.bgColorB, 0, 255);
  SetSliderValueFloat(hDlg, IDC_SLIDER_CAM_DIST, IDC_STATIC_CAM_DIST,
                      cfg.cameraDistance, 5.0f, 25.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_CAM_HEIGHT, IDC_STATIC_CAM_HEIGHT,
                      cfg.cameraHeight, 0.0f, 15.0f);
  SetSliderValue(hDlg, IDC_SLIDER_FOV, IDC_STATIC_FOV, (int)cfg.fieldOfView, 30,
                 90);
}

void ReadGeneralTab(HWND hDlg, Config &cfg) {
  cfg.quality = (QualityTier)SendMessage(GetDlgItem(hDlg, IDC_COMBO_QUALITY),
                                         CB_GETCURSEL, 0, 0);
  cfg.rotationSpeed =
      GetSliderValueFloat(hDlg, IDC_SLIDER_ROTATION, 0.0f, 2.0f);
  cfg.skyboxEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_SKYBOX) == BST_CHECKED;
  cfg.bgColorR = GetSliderValue(hDlg, IDC_SLIDER_BG_R);
  cfg.bgColorG = GetSliderValue(hDlg, IDC_SLIDER_BG_G);
  cfg.bgColorB = GetSliderValue(hDlg, IDC_SLIDER_BG_B);
  cfg.cameraDistance =
      GetSliderValueFloat(hDlg, IDC_SLIDER_CAM_DIST, 5.0f, 25.0f);
  cfg.cameraHeight =
      GetSliderValueFloat(hDlg, IDC_SLIDER_CAM_HEIGHT, 0.0f, 15.0f);
  cfg.fieldOfView = (float)GetSliderValue(hDlg, IDC_SLIDER_FOV);
}

bool HandleGeneralSliders(HWND hDlg, int sliderId) {
  switch (sliderId) {
  case IDC_SLIDER_ROTATION:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_ROTATION, true, 0.0f, 2.0f);
    return true;
  case IDC_SLIDER_BG_R:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BG_R, false, 0, 255);
    return true;
  case IDC_SLIDER_BG_G:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BG_G, false, 0, 255);
    return true;
  case IDC_SLIDER_BG_B:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BG_B, false, 0, 255);
    return true;
  case IDC_SLIDER_CAM_DIST:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CAM_DIST, true, 5.0f, 25.0f);
    return true;
  case IDC_SLIDER_CAM_HEIGHT:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CAM_HEIGHT, true, 0.0f, 15.0f);
    return true;
  case IDC_SLIDER_FOV:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FOV, false, 30, 90);
    return true;
  default:
    return false;
  }
}

} // namespace SettingsPanels
