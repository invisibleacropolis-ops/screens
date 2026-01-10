#pragma once

#include "../graphics/PostProcessConfig.h"
#include "ISettingsPanel.h"


// Layer Layout Panel - allows arranging visualizers on screen
// Shows a preview of layer positions and provides controls for resize/move
class LayerLayoutPanel : public ISettingsPanel {
public:
  const wchar_t *GetName() const override { return L"Layer Layout"; }

  int AddControls(LPWORD &p, int startY, int dialogWidth) override {
    return 0; // Controls defined in dialog resource
  }

  void InitFromConfig(HWND hDlg, const Config &cfg) override {
    // Initialize layer selection combo
    HWND hLayerCombo = GetDlgItem(hDlg, IDC_COMBO_LAYER_SELECT);
    if (hLayerCombo) {
      SendMessageW(hLayerCombo, CB_RESETCONTENT, 0, 0);
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"CPU");
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"RAM");
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"Disk");
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"Network");
      SendMessage(hLayerCombo, CB_SETCURSEL, 0, 0);
    }

    // Store config pointers for each layer
    m_transforms[0] = &m_cpuTransform;
    m_transforms[1] = &m_ramTransform;
    m_transforms[2] = &m_diskTransform;
    m_transforms[3] = &m_networkTransform;

    // Copy from config (would need to extend Config to include these)
    m_cpuTransform = LayerTransform::FullScreen();
    m_ramTransform = LayerTransform::FullScreen();
    m_diskTransform = LayerTransform::FullScreen();
    m_networkTransform = LayerTransform::FullScreen();

    // Load first layer's controls
    LoadLayerControls(hDlg, 0);

    // Initialize preset combo
    HWND hPresetCombo = GetDlgItem(hDlg, IDC_COMBO_LAYOUT_PRESET);
    if (hPresetCombo) {
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Full Screen");
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Top Left");
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Top Right");
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Bottom Left");
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Bottom Right");
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Center 50%");
      SendMessageW(hPresetCombo, CB_ADDSTRING, 0, (LPARAM)L"Quad Layout");
      SendMessage(hPresetCombo, CB_SETCURSEL, 0, 0);
    }
  }

  void ReadToConfig(HWND hDlg, Config &cfg) override {
    // Save current layer before reading
    SaveCurrentLayer(hDlg);

    // Would write transforms back to config
    // cfg.cpuLayer.transform = m_cpuTransform; etc.
  }

  bool HandleSlider(HWND hDlg, int sliderId) override {
    switch (sliderId) {
    case IDC_SLIDER_LAYER_X:
      UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_LAYER_X);
      return true;
    case IDC_SLIDER_LAYER_Y:
      UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_LAYER_Y);
      return true;
    case IDC_SLIDER_LAYER_W:
      UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_LAYER_W);
      return true;
    case IDC_SLIDER_LAYER_H:
      UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_LAYER_H);
      return true;
    case IDC_SLIDER_LAYER_DEPTH:
      UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_LAYER_DEPTH);
      return true;
    case IDC_SLIDER_LAYER_ROTATION:
      UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_LAYER_ROTATION);
      return true;
    }
    return false;
  }

  // Handle layer selection change
  void OnLayerSelected(HWND hDlg, int layerIndex) {
    SaveCurrentLayer(hDlg);
    m_currentLayer = layerIndex;
    LoadLayerControls(hDlg, layerIndex);
  }

  // Handle preset selection
  void OnPresetSelected(HWND hDlg, int presetIndex) {
    LayerTransform *t = m_transforms[m_currentLayer];
    if (!t)
      return;

    switch (presetIndex) {
    case 0:
      *t = LayerTransform::FullScreen();
      break;
    case 1:
      *t = LayerTransform::TopLeft();
      break;
    case 2:
      *t = LayerTransform::TopRight();
      break;
    case 3:
      *t = LayerTransform::BottomLeft();
      break;
    case 4:
      *t = LayerTransform::BottomRight();
      break;
    case 5:
      *t = LayerTransform::Centered(0.5f, 0.5f);
      break;
    case 6:
      ApplyQuadLayout();
      break;
    }
    LoadLayerControls(hDlg, m_currentLayer);
  }

private:
  void LoadLayerControls(HWND hDlg, int layerIndex) {
    LayerTransform *t = m_transforms[layerIndex];
    if (!t)
      return;

    // Position sliders (0-100 for 0.00-1.00)
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_X,
                                       IDC_STATIC_LAYER_X, t->x, 0.0f, 1.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_Y,
                                       IDC_STATIC_LAYER_Y, t->y, 0.0f, 1.0f);
    SettingsUtils::SetSliderValueFloat(
        hDlg, IDC_SLIDER_LAYER_W, IDC_STATIC_LAYER_W, t->width, 0.0f, 1.0f);
    SettingsUtils::SetSliderValueFloat(
        hDlg, IDC_SLIDER_LAYER_H, IDC_STATIC_LAYER_H, t->height, 0.0f, 1.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_DEPTH,
                                       IDC_STATIC_LAYER_DEPTH, t->depth, -1.0f,
                                       1.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_ROTATION,
                                       IDC_STATIC_LAYER_ROTATION, t->rotation,
                                       0.0f, 360.0f);

    CheckDlgButton(hDlg, IDC_CHECK_LAYER_VISIBLE,
                   t->visible ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_LAYER_LOCK_ASPECT,
                   t->lockAspect ? BST_CHECKED : BST_UNCHECKED);
  }

  void SaveCurrentLayer(HWND hDlg) {
    LayerTransform *t = m_transforms[m_currentLayer];
    if (!t)
      return;

    t->x = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_X, 0.0f,
                                              1.0f);
    t->y = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_Y, 0.0f,
                                              1.0f);
    t->width = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_W,
                                                  0.0f, 1.0f);
    t->height = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_H,
                                                   0.0f, 1.0f);
    t->depth = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_DEPTH,
                                                  -1.0f, 1.0f);
    t->rotation = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_LAYER_ROTATION, 0.0f, 360.0f);
    t->visible =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_LAYER_VISIBLE) == BST_CHECKED);
    t->lockAspect =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_LAYER_LOCK_ASPECT) == BST_CHECKED);
  }

  void UpdateSliderLabel(HWND hDlg, int sliderId, int labelId) {
    float value =
        SettingsUtils::GetSliderValueFloat(hDlg, sliderId, 0.0f, 1.0f);
    wchar_t buf[32];
    swprintf_s(buf, L"%.2f", value);
    SetDlgItemTextW(hDlg, labelId, buf);
  }

  void ApplyQuadLayout() {
    m_cpuTransform = LayerTransform::TopLeft();
    m_ramTransform = LayerTransform::TopRight();
    m_diskTransform = LayerTransform::BottomLeft();
    m_networkTransform = LayerTransform::BottomRight();
  }

  int m_currentLayer = 0;
  LayerTransform m_cpuTransform;
  LayerTransform m_ramTransform;
  LayerTransform m_diskTransform;
  LayerTransform m_networkTransform;
  LayerTransform *m_transforms[4] = {};
};
