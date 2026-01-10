#pragma once

#include "ISettingsPanel.h"

// Metrics panel - CPU, RAM, Disk, Network metric settings
class MetricsPanel : public ISettingsPanel {
public:
  const wchar_t *GetName() const override { return L"System Metrics"; }

  int AddControls(LPWORD &p, int startY, int dialogWidth) override {
    return 0; // Controls already in main dialog
  }

  void InitFromConfig(HWND hDlg, const Config &cfg) override {
    // CPU Metric
    InitMetricSection(hDlg, cfg.cpuMetric, IDC_CHECK_CPU_ENABLED,
                      IDC_SLIDER_CPU_THRESH, IDC_STATIC_CPU_THRESH,
                      IDC_SLIDER_CPU_STR, IDC_STATIC_CPU_STR,
                      IDC_COMBO_CPU_MESH);
    // RAM Metric
    InitMetricSection(hDlg, cfg.ramMetric, IDC_CHECK_RAM_ENABLED,
                      IDC_SLIDER_RAM_THRESH, IDC_STATIC_RAM_THRESH,
                      IDC_SLIDER_RAM_STR, IDC_STATIC_RAM_STR,
                      IDC_COMBO_RAM_MESH);
    // Disk Metric
    InitMetricSection(hDlg, cfg.diskMetric, IDC_CHECK_DISK_ENABLED,
                      IDC_SLIDER_DISK_THRESH, IDC_STATIC_DISK_THRESH,
                      IDC_SLIDER_DISK_STR, IDC_STATIC_DISK_STR,
                      IDC_COMBO_DISK_MESH);
    // Network Metric
    InitMetricSection(hDlg, cfg.networkMetric, IDC_CHECK_NET_ENABLED,
                      IDC_SLIDER_NET_THRESH, IDC_STATIC_NET_THRESH,
                      IDC_SLIDER_NET_STR, IDC_STATIC_NET_STR,
                      IDC_COMBO_NET_MESH);
  }

  void ReadToConfig(HWND hDlg, Config &cfg) override {
    ReadMetricSection(hDlg, cfg.cpuMetric, IDC_CHECK_CPU_ENABLED,
                      IDC_SLIDER_CPU_THRESH, IDC_SLIDER_CPU_STR,
                      IDC_COMBO_CPU_MESH);
    ReadMetricSection(hDlg, cfg.ramMetric, IDC_CHECK_RAM_ENABLED,
                      IDC_SLIDER_RAM_THRESH, IDC_SLIDER_RAM_STR,
                      IDC_COMBO_RAM_MESH);
    ReadMetricSection(hDlg, cfg.diskMetric, IDC_CHECK_DISK_ENABLED,
                      IDC_SLIDER_DISK_THRESH, IDC_SLIDER_DISK_STR,
                      IDC_COMBO_DISK_MESH);
    ReadMetricSection(hDlg, cfg.networkMetric, IDC_CHECK_NET_ENABLED,
                      IDC_SLIDER_NET_THRESH, IDC_SLIDER_NET_STR,
                      IDC_COMBO_NET_MESH);
  }

  bool HandleSlider(HWND hDlg, int sliderId) override {
    // CPU sliders
    if (sliderId == IDC_SLIDER_CPU_THRESH) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CPU_THRESH,
                                       0.0f, 100.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_CPU_STR) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CPU_STR, 0.0f,
                                       2.0f);
      return true;
    }
    // RAM sliders
    if (sliderId == IDC_SLIDER_RAM_THRESH) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_RAM_THRESH,
                                       0.0f, 100.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_RAM_STR) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_RAM_STR, 0.0f,
                                       2.0f);
      return true;
    }
    // Disk sliders
    if (sliderId == IDC_SLIDER_DISK_THRESH) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_DISK_THRESH,
                                       0.0f, 100.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_DISK_STR) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_DISK_STR,
                                       0.0f, 2.0f);
      return true;
    }
    // Network sliders
    if (sliderId == IDC_SLIDER_NET_THRESH) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_NET_THRESH,
                                       0.0f, 100.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_NET_STR) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_NET_STR, 0.0f,
                                       2.0f);
      return true;
    }
    return false;
  }

private:
  void InitMetricSection(HWND hDlg, const MetricConfig &metric, int checkId,
                         int threshSliderId, int threshLabelId, int strSliderId,
                         int strLabelId, int meshComboId) {
    CheckDlgButton(hDlg, checkId, metric.enabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, threshSliderId, threshLabelId,
                                       metric.threshold, 0.0f, 100.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, strSliderId, strLabelId,
                                       metric.strength, 0.0f, 2.0f);
    SettingsUtils::InitMeshCombo(hDlg, meshComboId, metric.meshType);
  }

  void ReadMetricSection(HWND hDlg, MetricConfig &metric, int checkId,
                         int threshSliderId, int strSliderId, int meshComboId) {
    metric.enabled = (IsDlgButtonChecked(hDlg, checkId) == BST_CHECKED);
    metric.threshold =
        SettingsUtils::GetSliderValueFloat(hDlg, threshSliderId, 0.0f, 100.0f);
    metric.strength =
        SettingsUtils::GetSliderValueFloat(hDlg, strSliderId, 0.0f, 2.0f);
    metric.meshType = (MeshType)SendMessage(GetDlgItem(hDlg, meshComboId),
                                            CB_GETCURSEL, 0, 0);
  }
};
