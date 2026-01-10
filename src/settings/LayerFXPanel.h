#pragma once

#include "../graphics/PostProcessConfig.h"
#include "ISettingsPanel.h"


// Layer FX Panel - per-layer post-processing effects settings
// Allows configuring bloom, glow, distortion, etc. for each visualizer
class LayerFXPanel : public ISettingsPanel {
public:
  const wchar_t *GetName() const override { return L"Layer Effects"; }

  int AddControls(LPWORD &p, int startY, int dialogWidth) override {
    return 0; // Controls defined in dialog resource
  }

  void InitFromConfig(HWND hDlg, const Config &cfg) override {
    // Initialize layer selection
    HWND hLayerCombo = GetDlgItem(hDlg, IDC_COMBO_FX_LAYER_SELECT);
    if (hLayerCombo) {
      SendMessageW(hLayerCombo, CB_RESETCONTENT, 0, 0);
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"CPU Effects");
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"RAM Effects");
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"Disk Effects");
      SendMessageW(hLayerCombo, CB_ADDSTRING, 0, (LPARAM)L"Network Effects");
      SendMessage(hLayerCombo, CB_SETCURSEL, 0, 0);
    }

    // Initialize with default presets
    m_fxConfigs[0] = FXPresets::CPUDefault();
    m_fxConfigs[1] = FXPresets::RAMDefault();
    m_fxConfigs[2] = FXPresets::DiskDefault();
    m_fxConfigs[3] = FXPresets::NetworkDefault();

    // Load first layer
    LoadFXControls(hDlg, 0);

    // Initialize blend mode combo
    HWND hBlendCombo = GetDlgItem(hDlg, IDC_COMBO_BLEND_MODE);
    if (hBlendCombo) {
      SendMessageW(hBlendCombo, CB_ADDSTRING, 0, (LPARAM)L"Additive");
      SendMessageW(hBlendCombo, CB_ADDSTRING, 0, (LPARAM)L"Multiply");
      SendMessageW(hBlendCombo, CB_ADDSTRING, 0, (LPARAM)L"Screen");
      SendMessageW(hBlendCombo, CB_ADDSTRING, 0, (LPARAM)L"Overlay");
      SendMessageW(hBlendCombo, CB_ADDSTRING, 0, (LPARAM)L"Normal");
    }
  }

  void ReadToConfig(HWND hDlg, Config &cfg) override {
    SaveCurrentFX(hDlg);
    // Would write FX configs back to main config
  }

  bool HandleSlider(HWND hDlg, int sliderId) override {
    // Bloom sliders
    if (sliderId == IDC_SLIDER_FX_BLOOM_THRESH) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId,
                                       IDC_STATIC_FX_BLOOM_THRESH, 0.0f, 2.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_FX_BLOOM_INT) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FX_BLOOM_INT,
                                       0.0f, 2.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_FX_BLOOM_RAD) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FX_BLOOM_RAD,
                                       1.0f, 20.0f);
      return true;
    }
    // Glow sliders
    if (sliderId == IDC_SLIDER_FX_GLOW_INT) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FX_GLOW_INT,
                                       0.0f, 2.0f);
      return true;
    }
    if (sliderId == IDC_SLIDER_FX_GLOW_SIZE) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FX_GLOW_SIZE,
                                       1.0f, 10.0f);
      return true;
    }
    // Opacity slider
    if (sliderId == IDC_SLIDER_FX_OPACITY) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FX_OPACITY,
                                       0.0f, 1.0f);
      return true;
    }
    // Chromatic sliders
    if (sliderId == IDC_SLIDER_FX_CHROM_OFFSET) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId,
                                       IDC_STATIC_FX_CHROM_OFFSET, 0.0f, 10.0f);
      return true;
    }
    // Distortion sliders
    if (sliderId == IDC_SLIDER_FX_DISTORT_AMT) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId,
                                       IDC_STATIC_FX_DISTORT_AMT, 0.0f, 1.0f);
      return true;
    }
    // Trail slider
    if (sliderId == IDC_SLIDER_FX_TRAIL_FADE) {
      SettingsUtils::UpdateSliderLabel(hDlg, sliderId, IDC_STATIC_FX_TRAIL_FADE,
                                       0.0f, 1.0f);
      return true;
    }
    return false;
  }

  void OnLayerSelected(HWND hDlg, int layerIndex) {
    SaveCurrentFX(hDlg);
    m_currentLayer = layerIndex;
    LoadFXControls(hDlg, layerIndex);
  }

private:
  void LoadFXControls(HWND hDlg, int layerIndex) {
    const PostProcessConfig &fx = m_fxConfigs[layerIndex];

    // Bloom
    CheckDlgButton(hDlg, IDC_CHECK_FX_BLOOM,
                   fx.bloomEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_THRESH,
                                       IDC_STATIC_FX_BLOOM_THRESH,
                                       fx.bloomThreshold, 0.0f, 2.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_INT,
                                       IDC_STATIC_FX_BLOOM_INT,
                                       fx.bloomIntensity, 0.0f, 2.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_RAD,
                                       IDC_STATIC_FX_BLOOM_RAD, fx.bloomRadius,
                                       1.0f, 20.0f);

    // Glow
    CheckDlgButton(hDlg, IDC_CHECK_FX_GLOW,
                   fx.glowEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_GLOW_INT,
                                       IDC_STATIC_FX_GLOW_INT, fx.glowIntensity,
                                       0.0f, 2.0f);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_GLOW_SIZE,
                                       IDC_STATIC_FX_GLOW_SIZE, fx.glowSize,
                                       1.0f, 10.0f);

    // Chromatic
    CheckDlgButton(hDlg, IDC_CHECK_FX_CHROM,
                   fx.chromaticEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_CHROM_OFFSET,
                                       IDC_STATIC_FX_CHROM_OFFSET,
                                       fx.chromaticOffset, 0.0f, 10.0f);

    // Distortion
    CheckDlgButton(hDlg, IDC_CHECK_FX_DISTORT,
                   fx.distortionEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_DISTORT_AMT,
                                       IDC_STATIC_FX_DISTORT_AMT,
                                       fx.distortionAmount, 0.0f, 1.0f);

    // Trails
    CheckDlgButton(hDlg, IDC_CHECK_FX_TRAILS,
                   fx.trailsEnabled ? BST_CHECKED : BST_UNCHECKED);
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_TRAIL_FADE,
                                       IDC_STATIC_FX_TRAIL_FADE, fx.trailsFade,
                                       0.0f, 1.0f);

    // Scan lines
    CheckDlgButton(hDlg, IDC_CHECK_FX_SCANLINES,
                   fx.scanLinesEnabled ? BST_CHECKED : BST_UNCHECKED);

    // Noise
    CheckDlgButton(hDlg, IDC_CHECK_FX_NOISE,
                   fx.noiseEnabled ? BST_CHECKED : BST_UNCHECKED);

    // Pixelate
    CheckDlgButton(hDlg, IDC_CHECK_FX_PIXELATE,
                   fx.pixelateEnabled ? BST_CHECKED : BST_UNCHECKED);

    // Motion blur
    CheckDlgButton(hDlg, IDC_CHECK_FX_MOTIONBLUR,
                   fx.motionBlurEnabled ? BST_CHECKED : BST_UNCHECKED);

    // Opacity and blend
    SettingsUtils::SetSliderValueFloat(hDlg, IDC_SLIDER_FX_OPACITY,
                                       IDC_STATIC_FX_OPACITY, fx.opacity, 0.0f,
                                       1.0f);
    SendMessage(GetDlgItem(hDlg, IDC_COMBO_BLEND_MODE), CB_SETCURSEL,
                (int)fx.blendMode, 0);
  }

  void SaveCurrentFX(HWND hDlg) {
    PostProcessConfig &fx = m_fxConfigs[m_currentLayer];

    fx.bloomEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_BLOOM) == BST_CHECKED);
    fx.bloomThreshold = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_BLOOM_THRESH, 0.0f, 2.0f);
    fx.bloomIntensity = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_BLOOM_INT, 0.0f, 2.0f);
    fx.bloomRadius = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_BLOOM_RAD, 1.0f, 20.0f);

    fx.glowEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_GLOW) == BST_CHECKED);
    fx.glowIntensity = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_GLOW_INT, 0.0f, 2.0f);
    fx.glowSize = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_GLOW_SIZE, 1.0f, 10.0f);

    fx.chromaticEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_CHROM) == BST_CHECKED);
    fx.chromaticOffset = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_CHROM_OFFSET, 0.0f, 10.0f);

    fx.distortionEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_DISTORT) == BST_CHECKED);
    fx.distortionAmount = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_DISTORT_AMT, 0.0f, 1.0f);

    fx.trailsEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_TRAILS) == BST_CHECKED);
    fx.trailsFade = SettingsUtils::GetSliderValueFloat(
        hDlg, IDC_SLIDER_FX_TRAIL_FADE, 0.0f, 1.0f);

    fx.scanLinesEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_SCANLINES) == BST_CHECKED);
    fx.noiseEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_NOISE) == BST_CHECKED);
    fx.pixelateEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_PIXELATE) == BST_CHECKED);
    fx.motionBlurEnabled =
        (IsDlgButtonChecked(hDlg, IDC_CHECK_FX_MOTIONBLUR) == BST_CHECKED);

    fx.opacity = SettingsUtils::GetSliderValueFloat(hDlg, IDC_SLIDER_FX_OPACITY,
                                                    0.0f, 1.0f);
    fx.blendMode = (BlendMode)SendMessage(
        GetDlgItem(hDlg, IDC_COMBO_BLEND_MODE), CB_GETCURSEL, 0, 0);
  }

  int m_currentLayer = 0;
  PostProcessConfig m_fxConfigs[4]; // CPU, RAM, Disk, Network
};
