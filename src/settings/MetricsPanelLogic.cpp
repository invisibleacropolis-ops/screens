#include "MetricsPanelLogic.h"

#include "../resource.h"
#include <commctrl.h>

namespace {

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

void InitMeshCombo(HWND hDlg, int comboId, MeshType selected) {
  HWND hCombo = GetDlgItem(hDlg, comboId);
  SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Sphere");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Cube");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Ring");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"None");
  SendMessage(hCombo, CB_SETCURSEL, (int)selected, 0);
}

void InitMetricControls(HWND hDlg, const MetricConfig &metric, int checkId,
                        int threshSliderId, int threshLabelId, int strSliderId,
                        int strLabelId, int meshComboId) {
  CheckDlgButton(hDlg, checkId, metric.enabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, threshSliderId, threshLabelId, metric.threshold,
                      0.0f, 100.0f);
  SetSliderValueFloat(hDlg, strSliderId, strLabelId, metric.strength, 0.0f,
                      2.0f);
  InitMeshCombo(hDlg, meshComboId, metric.meshType);
}

void ReadMetricControls(HWND hDlg, MetricConfig &metric, int checkId,
                        int threshSliderId, int strSliderId, int meshComboId) {
  metric.enabled = (IsDlgButtonChecked(hDlg, checkId) == BST_CHECKED);
  metric.threshold = GetSliderValueFloat(hDlg, threshSliderId, 0.0f, 100.0f);
  metric.strength = GetSliderValueFloat(hDlg, strSliderId, 0.0f, 2.0f);
  metric.meshType =
      (MeshType)SendMessage(GetDlgItem(hDlg, meshComboId), CB_GETCURSEL, 0, 0);
}

} // namespace

namespace SettingsPanels {

void InitMetricsTab(HWND hDlg, const Config &cfg) {
  InitMetricControls(hDlg, cfg.cpuMetric, IDC_CHECK_CPU_ENABLED,
                     IDC_SLIDER_CPU_THRESHOLD, IDC_STATIC_CPU_THRESHOLD,
                     IDC_SLIDER_CPU_STRENGTH, IDC_STATIC_CPU_STRENGTH,
                     IDC_COMBO_CPU_MESH);

  SetSliderValueFloat(hDlg, IDC_SLIDER_CPU_Y, IDC_STATIC_CPU_Y, cfg.cpuYOffset,
                      -10.0f, 10.0f);
  HWND hCpuGrid = GetDlgItem(hDlg, IDC_SLIDER_CPU_GRID);
  SendMessage(hCpuGrid, TBM_SETRANGE, TRUE, MAKELPARAM(20, 200));
  SendMessage(hCpuGrid, TBM_SETPOS, TRUE, cfg.cpuGridSize);
  wchar_t buf[32];
  swprintf_s(buf, L"%d", cfg.cpuGridSize);
  SetDlgItemTextW(hDlg, IDC_STATIC_CPU_GRID, buf);
  CheckDlgButton(hDlg, IDC_CHECK_CPU_SPECTRUM,
                 cfg.cpuSpectrum ? BST_CHECKED : BST_UNCHECKED);

  InitMetricControls(hDlg, cfg.ramMetric, IDC_CHECK_RAM_ENABLED,
                     IDC_SLIDER_RAM_THRESHOLD, IDC_STATIC_RAM_THRESHOLD,
                     IDC_SLIDER_RAM_STRENGTH, IDC_STATIC_RAM_STRENGTH,
                     IDC_COMBO_RAM_MESH);
  InitMetricControls(hDlg, cfg.diskMetric, IDC_CHECK_DISK_ENABLED,
                     IDC_SLIDER_DISK_THRESHOLD, IDC_STATIC_DISK_THRESHOLD,
                     IDC_SLIDER_DISK_STRENGTH, IDC_STATIC_DISK_STRENGTH,
                     IDC_COMBO_DISK_MESH);
  InitMetricControls(hDlg, cfg.networkMetric, IDC_CHECK_NET_ENABLED,
                     IDC_SLIDER_NET_THRESHOLD, IDC_STATIC_NET_THRESHOLD,
                     IDC_SLIDER_NET_STRENGTH, IDC_STATIC_NET_STRENGTH,
                     IDC_COMBO_NET_MESH);
}

void ReadMetricsTab(HWND hDlg, Config &cfg) {
  ReadMetricControls(hDlg, cfg.cpuMetric, IDC_CHECK_CPU_ENABLED,
                     IDC_SLIDER_CPU_THRESHOLD, IDC_SLIDER_CPU_STRENGTH,
                     IDC_COMBO_CPU_MESH);
  cfg.cpuGridSize = GetSliderValue(hDlg, IDC_SLIDER_CPU_GRID);
  cfg.cpuYOffset = GetSliderValueFloat(hDlg, IDC_SLIDER_CPU_Y, -10.0f, 10.0f);
  cfg.cpuSpectrum = IsDlgButtonChecked(hDlg, IDC_CHECK_CPU_SPECTRUM) == BST_CHECKED;

  ReadMetricControls(hDlg, cfg.ramMetric, IDC_CHECK_RAM_ENABLED,
                     IDC_SLIDER_RAM_THRESHOLD, IDC_SLIDER_RAM_STRENGTH,
                     IDC_COMBO_RAM_MESH);
  ReadMetricControls(hDlg, cfg.diskMetric, IDC_CHECK_DISK_ENABLED,
                     IDC_SLIDER_DISK_THRESHOLD, IDC_SLIDER_DISK_STRENGTH,
                     IDC_COMBO_DISK_MESH);
  ReadMetricControls(hDlg, cfg.networkMetric, IDC_CHECK_NET_ENABLED,
                     IDC_SLIDER_NET_THRESHOLD, IDC_SLIDER_NET_STRENGTH,
                     IDC_COMBO_NET_MESH);
}

bool HandleMetricsSliders(HWND hDlg, int sliderId) {
  switch (sliderId) {
  case IDC_SLIDER_CPU_THRESHOLD:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CPU_THRESHOLD, true, 0.0f,
                      100.0f);
    return true;
  case IDC_SLIDER_CPU_STRENGTH:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CPU_STRENGTH, true, 0.0f,
                      2.0f);
    return true;
  case IDC_SLIDER_CPU_GRID:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CPU_GRID, false, 20, 200);
    return true;
  case IDC_SLIDER_CPU_Y:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CPU_Y, true, -10.0f, 10.0f);
    return true;
  case IDC_SLIDER_RAM_THRESHOLD:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_RAM_THRESHOLD, true, 0.0f,
                      100.0f);
    return true;
  case IDC_SLIDER_RAM_STRENGTH:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_RAM_STRENGTH, true, 0.0f,
                      2.0f);
    return true;
  case IDC_SLIDER_DISK_THRESHOLD:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_DISK_THRESHOLD, true, 0.0f,
                      100.0f);
    return true;
  case IDC_SLIDER_DISK_STRENGTH:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_DISK_STRENGTH, true, 0.0f,
                      2.0f);
    return true;
  case IDC_SLIDER_NET_THRESHOLD:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_NET_THRESHOLD, true, 0.0f,
                      100.0f);
    return true;
  case IDC_SLIDER_NET_STRENGTH:
    UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_NET_STRENGTH, true, 0.0f,
                      2.0f);
    return true;
  default:
    return false;
  }
}

} // namespace SettingsPanels
