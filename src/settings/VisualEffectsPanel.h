#pragma once

#include "ISettingsPanel.h"

// Visual Effects panel - Bloom, FXAA, Exposure settings
class VisualEffectsPanel : public ISettingsPanel {
public:
  const wchar_t *GetName() const override { return L"Visual Effects"; }

  int AddControls(LPWORD &p, int startY, int dialogWidth) override {
    // This panel's controls are already in the main dialog
    // In future refactoring, control creation would be moved here
    return 0;
  }

  void InitFromConfig(HWND hDlg, const Config &cfg) override {
    CheckDlgButton(hDlg, IDC_CHECK_BLOOM,
                   cfg.bloomEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_THRESH,
                                       IDC_STATIC_BLOOM_THRESH,
                                       cfg.bloomThreshold, 0.3f, 1.5f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_STR,
                                       IDC_STATIC_BLOOM_STR, cfg.bloomStrength,
                                       0.0f, 2.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_EXPOSURE,
                                       IDC_STATIC_EXPOSURE, cfg.exposure, 0.5f,
                                       3.0f);
    CheckDlgButton(hDlg, IDC_CHECK_FXAA,
                   cfg.fxaaEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FOG, IDC_STATIC_FOG,
                                       cfg.fogDensity, 0.0f, 1.0f);
  }

  void ReadToConfig(HWND hDlg, Config &cfg) override {
    cfg.bloomEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_BLOOM) == BST_CHECKED);
    cfg.bloomThreshold = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_BLOOM_THRESH, 0.3f, 1.5f);
    cfg.bloomStrength = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_BLOOM_STR, 0.0f, 2.0f);
    cfg.exposure = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_EXPOSURE,
                                                      0.5f, 3.0f);
    cfg.fxaaEnabled = (IsDlgButtonChecked(hDlg, IDC_CHECK_FXAA) == BST_CHECKED);
    cfg.fogDensity =
        SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_FOG, 0.0f, 1.0f);
  }

  bool HandleSlider(HWND hDlg, int sliderId) override {
    switch (sliderId) {
    case IDC_SLIDER_BLOOM_THRESH:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BLOOM_THRESH,
                                       0.3f, 1.5f);
      return true;
    case IDC_SLIDER_BLOOM_STR:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_BLOOM_STR,
                                       0.0f, 2.0f);
      return true;
    case IDC_SLIDER_EXPOSURE:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_EXPOSURE,
                                       0.5f, 3.0f);
      return true;
    case IDC_SLIDER_FOG:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FOG, 0.0f,
                                       1.0f);
      return true;
    }
    return false;
  }
};
