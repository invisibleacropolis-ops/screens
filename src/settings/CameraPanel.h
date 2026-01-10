#pragma once

#include "ISettingsPanel.h"

// Camera panel - FOV, Distance, Height, Rotation settings
class CameraPanel : public ISettingsPanel {
public:
  const wchar_t *GetName() const override { return L"Camera"; }

  int AddControls(LPWORD &p, int startY, int dialogWidth) override {
    return 0; // Controls already in main dialog
  }

  void InitFromConfig(HWND hDlg, const Config &cfg) override {
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_CAM_DIST,
                                       IDC_STATIC_CAM_DIST, cfg.cameraDistance,
                                       3.0f, 20.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_CAM_HEIGHT,
                                       IDC_STATIC_CAM_HEIGHT, cfg.cameraHeight,
                                       0.0f, 10.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FOV, IDC_STATIC_FOV,
                                       cfg.fov, 30.0f, 120.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_ROT_SPEED,
                                       IDC_STATIC_ROT_SPEED, cfg.rotationSpeed,
                                       0.0f, 50.0f);
  }

  void ReadToConfig(HWND hDlg, Config &cfg) override {
    cfg.cameraDistance = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_CAM_DIST, 3.0f, 20.0f);
    cfg.cameraHeight = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_CAM_HEIGHT, 0.0f, 10.0f);
    cfg.fov =
        SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_FOV, 30.0f, 120.0f);
    cfg.rotationSpeed = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_ROT_SPEED, 0.0f, 50.0f);
  }

  bool HandleSlider(HWND hDlg, int sliderId) override {
    switch (sliderId) {
    case IDC_SLIDER_CAM_DIST:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CAM_DIST,
                                       3.0f, 20.0f);
      return true;
    case IDC_SLIDER_CAM_HEIGHT:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_CAM_HEIGHT,
                                       0.0f, 10.0f);
      return true;
    case IDC_SLIDER_FOV:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FOV, 30.0f,
                                       120.0f);
      return true;
    case IDC_SLIDER_ROT_SPEED:
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_ROT_SPEED,
                                       0.0f, 50.0f);
      return true;
    }
    return false;
  }
};
