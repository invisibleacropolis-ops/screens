#include "SettingsDialog.h"
#include "resource.h"
#include <commctrl.h>
#include <cstdio>

#pragma comment(lib, "comctl32.lib")

// Global config pointer for dialog proc
static Config *g_pConfig = nullptr;
static Config g_tempConfig;
static int g_currentLayoutLayer = 0;
static int g_currentFXLayer = 0;

// Helper: Set slider value and update label
static void SetSliderValue(HWND hDlg, int sliderId, int labelId, int value,
                           int minVal, int maxVal) {
  HWND hSlider = GetDlgItem(hDlg, sliderId);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
  SendMessage(hSlider, TBM_SETPOS, TRUE, value);

  wchar_t buf[32];
  swprintf_s(buf, L"%d", value);
  SetDlgItemTextW(hDlg, labelId, buf);
}

static void SetSliderValueFloat(HWND hDlg, int sliderId, int labelId,
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

static int GetSliderValue(HWND hDlg, int sliderId) {
  return (int)SendMessage(GetDlgItem(hDlg, sliderId), TBM_GETPOS, 0, 0);
}

static float GetSliderValueFloat(HWND hDlg, int sliderId, float minVal,
                                 float maxVal, int steps = 100) {
  int pos = GetSliderValue(hDlg, sliderId);
  return minVal + (float)pos / steps * (maxVal - minVal);
}

// Helper to initialize mesh combo box
static void InitMeshCombo(HWND hDlg, int comboId, MeshType selected) {
  HWND hCombo = GetDlgItem(hDlg, comboId);
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Sphere");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Cube");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Ring");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"None");
  SendMessage(hCombo, CB_SETCURSEL, (int)selected, 0);
}

// Helper to initialize a metric section
static void InitMetricControls(HWND hDlg, const MetricConfig &metric,
                               int checkId, int threshSliderId,
                               int threshLabelId, int strSliderId,
                               int strLabelId, int meshComboId) {
  CheckDlgButton(hDlg, checkId, metric.enabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, threshSliderId, threshLabelId, metric.threshold,
                      0.0f, 100.0f);
  SetSliderValueFloat(hDlg, strSliderId, strLabelId, metric.strength, 0.0f,
                      2.0f);
  InitMeshCombo(hDlg, meshComboId, metric.meshType);
}

// Helper to read a metric section
static void ReadMetricControls(HWND hDlg, MetricConfig &metric, int checkId,
                               int threshSliderId, int strSliderId,
                               int meshComboId) {
  metric.enabled = (IsDlgButtonChecked(hDlg, checkId) == BST_CHECKED);
  metric.threshold = GetSliderValueFloat(hDlg, threshSliderId, 0.0f, 100.0f);
  metric.strength = GetSliderValueFloat(hDlg, strSliderId, 0.0f, 2.0f);
  metric.meshType =
      (MeshType)SendMessage(GetDlgItem(hDlg, meshComboId), CB_GETCURSEL, 0, 0);
}

// Helper to load layer layout controls
static void LoadLayerControls(HWND hDlg, int layerIndex) {
  if (layerIndex < 0 || layerIndex >= 4)
    return;
  const auto &t = g_tempConfig.layerConfigs[layerIndex].transform;

  SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_X, IDC_STATIC_LAYER_X, t.x, 0.0f,
                      1.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_Y, IDC_STATIC_LAYER_Y, t.y, 0.0f,
                      1.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_W, IDC_STATIC_LAYER_W, t.width,
                      0.0f, 1.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_H, IDC_STATIC_LAYER_H, t.height,
                      0.0f, 1.0f);
  CheckDlgButton(hDlg, IDC_CHECK_LAYER_VISIBLE,
                 t.visible ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_CHECK_LAYER_LOCK_ASPECT,
                 t.lockAspect ? BST_CHECKED : BST_UNCHECKED);
}

// Helper to save layer layout controls
static void SaveLayerControls(HWND hDlg, int layerIndex) {
  if (layerIndex < 0 || layerIndex >= 4)
    return;
  auto &t = g_tempConfig.layerConfigs[layerIndex].transform;

  t.x = GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_X, 0.0f, 1.0f);
  t.y = GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_Y, 0.0f, 1.0f);
  t.width = GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_W, 0.0f, 1.0f);
  t.height = GetSliderValueFloat(hDlg, IDC_SLIDER_LAYER_H, 0.0f, 1.0f);
  t.visible = IsDlgButtonChecked(hDlg, IDC_CHECK_LAYER_VISIBLE) == BST_CHECKED;
  t.lockAspect =
      IsDlgButtonChecked(hDlg, IDC_CHECK_LAYER_LOCK_ASPECT) == BST_CHECKED;
}

// Helper to load layer FX controls
static void LoadFXControls(HWND hDlg, int layerIndex) {
  if (layerIndex < 0 || layerIndex >= 4)
    return;
  const auto &fx = g_tempConfig.layerConfigs[layerIndex];

  SendMessage(GetDlgItem(hDlg, IDC_COMBO_BLEND_MODE), CB_SETCURSEL,
              (int)fx.blendMode, 0);

  CheckDlgButton(hDlg, IDC_CHECK_FX_BLOOM,
                 fx.bloomEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_INT, IDC_STATIC_FX_BLOOM_INT,
                      fx.bloomIntensity, 0.0f, 2.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_THRESH,
                      IDC_STATIC_FX_BLOOM_THRESH, fx.bloomThreshold, 0.0f,
                      2.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_RAD, IDC_STATIC_FX_BLOOM_RAD,
                      fx.bloomRadius, 1.0f, 20.0f);

  CheckDlgButton(hDlg, IDC_CHECK_FX_GLOW,
                 fx.glowEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_GLOW_INT, IDC_STATIC_FX_GLOW_INT,
                      fx.glowIntensity, 0.0f, 2.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_GLOW_SIZE, IDC_STATIC_FX_GLOW_SIZE,
                      fx.glowSize, 1.0f, 10.0f);

  CheckDlgButton(hDlg, IDC_CHECK_FX_CHROM,
                 fx.chromaticEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_CHROM_OFFSET,
                      IDC_STATIC_FX_CHROM_OFFSET, fx.chromaticOffset, 0.0f,
                      10.0f);

  CheckDlgButton(hDlg, IDC_CHECK_FX_DISTORT,
                 fx.distortionEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_DISTORT_AMT,
                      IDC_STATIC_FX_DISTORT_AMT, fx.distortionAmount, 0.0f,
                      1.0f);

  CheckDlgButton(hDlg, IDC_CHECK_FX_TRAILS,
                 fx.trailsEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_TRAIL_FADE, IDC_STATIC_FX_TRAIL_FADE,
                      fx.trailsFade, 0.0f, 1.0f);

  CheckDlgButton(hDlg, IDC_CHECK_FX_SCANLINES,
                 fx.scanLinesEnabled ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_CHECK_FX_NOISE,
                 fx.noiseEnabled ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_CHECK_FX_MOTIONBLUR,
                 fx.motionBlurEnabled ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_CHECK_FX_PIXELATE,
                 fx.pixelateEnabled ? BST_CHECKED : BST_UNCHECKED);

  SetSliderValueFloat(hDlg, IDC_SLIDER_FX_OPACITY, IDC_STATIC_FX_OPACITY,
                      fx.opacity, 0.0f, 1.0f);
}

// Helper to save layer FX controls
static void SaveFXControls(HWND hDlg, int layerIndex) {
  if (layerIndex < 0 || layerIndex >= 4)
    return;
  auto &fx = g_tempConfig.layerConfigs[layerIndex];

  fx.blendMode = (BlendMode)SendMessage(GetDlgItem(hDlg, IDC_COMBO_BLEND_MODE),
                                        CB_GETCURSEL, 0, 0);

  fx.bloomEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_FX_BLOOM) == BST_CHECKED;
  fx.bloomIntensity =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_INT, 0.0f, 2.0f);
  fx.bloomThreshold =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_THRESH, 0.0f, 2.0f);
  fx.bloomRadius =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_BLOOM_RAD, 1.0f, 20.0f);

  fx.glowEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_FX_GLOW) == BST_CHECKED;
  fx.glowIntensity =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_GLOW_INT, 0.0f, 2.0f);
  fx.glowSize = GetSliderValueFloat(hDlg, IDC_SLIDER_FX_GLOW_SIZE, 1.0f, 10.0f);

  fx.chromaticEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_FX_CHROM) == BST_CHECKED;
  fx.chromaticOffset =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_CHROM_OFFSET, 0.0f, 10.0f);

  fx.distortionEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_FX_DISTORT) == BST_CHECKED;
  fx.distortionAmount =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_DISTORT_AMT, 0.0f, 1.0f);

  fx.trailsEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_FX_TRAILS) == BST_CHECKED;
  fx.trailsFade =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FX_TRAIL_FADE, 0.0f, 1.0f);

  fx.scanLinesEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_FX_SCANLINES) == BST_CHECKED;
  fx.noiseEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_FX_NOISE) == BST_CHECKED;
  fx.motionBlurEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_FX_MOTIONBLUR) == BST_CHECKED;
  fx.pixelateEnabled =
      IsDlgButtonChecked(hDlg, IDC_CHECK_FX_PIXELATE) == BST_CHECKED;

  fx.opacity = GetSliderValueFloat(hDlg, IDC_SLIDER_FX_OPACITY, 0.0f, 1.0f);
}

// Initialize dialog controls from config
static void InitDialogFromConfig(HWND hDlg, const Config &cfg) {
  // Ensure layer configs have defaults if not set
  if (cfg.layerConfigs[0].transform.width == 0) {
    // CPU - Top Left / Full depending on preference, let's default to a quad
    // layout for now Actually, let's use the presets But since we can't easily
    // access presets here, let's just ensure defaults This part might be better
    // in Config.cpp or LoadConfigConfig
  }

  // Quality combo
  HWND hQuality = GetDlgItem(hDlg, IDC_COMBO_QUALITY);
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"Low");
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"Medium");
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"High");
  SendMessage(hQuality, CB_SETCURSEL, (int)cfg.quality, 0);

  // Visual Effects
  CheckDlgButton(hDlg, IDC_CHECK_BLOOM,
                 cfg.bloomEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_THRESH, IDC_STATIC_BLOOM_THRESH,
                      cfg.bloomThreshold, 0.3f, 1.5f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_STR, IDC_STATIC_BLOOM_STR,
                      cfg.bloomStrength, 0.0f, 2.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_EXPOSURE, IDC_STATIC_EXPOSURE,
                      cfg.exposure, 0.5f, 3.0f);
  CheckDlgButton(hDlg, IDC_CHECK_FXAA,
                 cfg.fxaaEnabled ? BST_CHECKED : BST_UNCHECKED);

  // Fog
  CheckDlgButton(hDlg, IDC_CHECK_FOG,
                 cfg.fogEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValueFloat(hDlg, IDC_SLIDER_FOG_DENSITY, IDC_STATIC_FOG_DENSITY,
                      cfg.fogDensity, 0.0f, 0.2f);

  // Particles
  CheckDlgButton(hDlg, IDC_CHECK_PARTICLES,
                 cfg.particlesEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValue(hDlg, IDC_SLIDER_PARTICLE_CNT, IDC_STATIC_PARTICLE_CNT,
                 cfg.particleCount, 500, 10000);

  // Scene
  SetSliderValueFloat(hDlg, IDC_SLIDER_ROTATION, IDC_STATIC_ROTATION,
                      cfg.rotationSpeed, 0.0f, 2.0f);

  // Camera
  SetSliderValueFloat(hDlg, IDC_SLIDER_CAM_DIST, IDC_STATIC_CAM_DIST,
                      cfg.cameraDistance, 5.0f, 25.0f);
  SetSliderValueFloat(hDlg, IDC_SLIDER_CAM_HEIGHT, IDC_STATIC_CAM_HEIGHT,
                      cfg.cameraHeight, 0.0f, 15.0f);
  SetSliderValue(hDlg, IDC_SLIDER_FOV, IDC_STATIC_FOV, (int)cfg.fieldOfView, 30,
                 90);

  // Background
  CheckDlgButton(hDlg, IDC_CHECK_SKYBOX,
                 cfg.skyboxEnabled ? BST_CHECKED : BST_UNCHECKED);
  SetSliderValue(hDlg, IDC_SLIDER_BG_R, IDC_STATIC_BG_R, cfg.bgColorR, 0, 255);
  SetSliderValue(hDlg, IDC_SLIDER_BG_G, IDC_STATIC_BG_G, cfg.bgColorG, 0, 255);
  SetSliderValue(hDlg, IDC_SLIDER_BG_B, IDC_STATIC_BG_B, cfg.bgColorB, 0, 255);

  // Metrics
  InitMetricControls(hDlg, cfg.cpuMetric, IDC_CHECK_CPU_ENABLED,
                     IDC_SLIDER_CPU_THRESHOLD, IDC_STATIC_CPU_THRESHOLD,
                     IDC_SLIDER_CPU_STRENGTH, IDC_STATIC_CPU_STRENGTH,
                     IDC_COMBO_CPU_MESH);
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

  // Layer Layout combos
  HWND hLayerSelect = GetDlgItem(hDlg, IDC_COMBO_LAYER_SELECT);
  SendMessageW(hLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"CPU");
  SendMessageW(hLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"RAM");
  SendMessageW(hLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"Disk");
  SendMessageW(hLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"Network");
  SendMessage(hLayerSelect, CB_SETCURSEL, 0, 0);

  HWND hPreset = GetDlgItem(hDlg, IDC_COMBO_LAYOUT_PRESET);
  SendMessageW(hPreset, CB_ADDSTRING, 0, (LPARAM)L"Full Screen");
  SendMessageW(hPreset, CB_ADDSTRING, 0, (LPARAM)L"Top Left");
  SendMessageW(hPreset, CB_ADDSTRING, 0, (LPARAM)L"Top Right");
  SendMessageW(hPreset, CB_ADDSTRING, 0, (LPARAM)L"Bottom Left");
  SendMessageW(hPreset, CB_ADDSTRING, 0, (LPARAM)L"Bottom Right");
  SendMessageW(hPreset, CB_ADDSTRING, 0, (LPARAM)L"Quad Layout");
  SendMessage(hPreset, CB_SETCURSEL, 0, 0);

  // Initialize Layout Controls for Layer 0
  g_currentLayoutLayer = 0;
  LoadLayerControls(hDlg, 0);

  // Layer FX combos
  HWND hFXLayerSelect = GetDlgItem(hDlg, IDC_COMBO_FX_LAYER_SELECT);
  SendMessageW(hFXLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"CPU");
  SendMessageW(hFXLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"RAM");
  SendMessageW(hFXLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"Disk");
  SendMessageW(hFXLayerSelect, CB_ADDSTRING, 0, (LPARAM)L"Network");
  SendMessage(hFXLayerSelect, CB_SETCURSEL, 0, 0);

  HWND hBlendMode = GetDlgItem(hDlg, IDC_COMBO_BLEND_MODE);
  SendMessageW(hBlendMode, CB_ADDSTRING, 0, (LPARAM)L"Additive");
  SendMessageW(hBlendMode, CB_ADDSTRING, 0, (LPARAM)L"Multiply");
  SendMessageW(hBlendMode, CB_ADDSTRING, 0, (LPARAM)L"Screen");
  SendMessageW(hBlendMode, CB_ADDSTRING, 0, (LPARAM)L"Normal");
  SendMessage(hBlendMode, CB_SETCURSEL, 0, 0);

  // Initialize FX Controls for Layer 0
  g_currentFXLayer = 0;
  LoadFXControls(hDlg, 0);
}

// Read config from dialog controls
static void ReadConfigFromDialog(HWND hDlg, Config &cfg) {
  // Save current layer adjustments
  SaveLayerControls(hDlg, g_currentLayoutLayer);
  SaveFXControls(hDlg, g_currentFXLayer);

  // Copy all layer configs from temp to output
  cfg.layerConfigs = g_tempConfig.layerConfigs;

  // Quality
  cfg.quality = (QualityTier)SendMessage(GetDlgItem(hDlg, IDC_COMBO_QUALITY),
                                         CB_GETCURSEL, 0, 0);

  // Visual Effects
  cfg.bloomEnabled = (IsDlgButtonChecked(hDlg, IDC_CHECK_BLOOM) == BST_CHECKED);
  cfg.bloomThreshold =
      GetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_THRESH, 0.3f, 1.5f);
  cfg.bloomStrength =
      GetSliderValueFloat(hDlg, IDC_SLIDER_BLOOM_STR, 0.0f, 2.0f);
  cfg.exposure = GetSliderValueFloat(hDlg, IDC_SLIDER_EXPOSURE, 0.5f, 3.0f);
  cfg.fxaaEnabled = (IsDlgButtonChecked(hDlg, IDC_CHECK_FXAA) == BST_CHECKED);

  // Fog
  cfg.fogEnabled = (IsDlgButtonChecked(hDlg, IDC_CHECK_FOG) == BST_CHECKED);
  cfg.fogDensity =
      GetSliderValueFloat(hDlg, IDC_SLIDER_FOG_DENSITY, 0.0f, 0.2f);

  // Particles
  cfg.particlesEnabled =
      (IsDlgButtonChecked(hDlg, IDC_CHECK_PARTICLES) == BST_CHECKED);
  cfg.particleCount = GetSliderValue(hDlg, IDC_SLIDER_PARTICLE_CNT);

  // Scene
  cfg.rotationSpeed =
      GetSliderValueFloat(hDlg, IDC_SLIDER_ROTATION, 0.0f, 2.0f);

  // Camera
  cfg.cameraDistance =
      GetSliderValueFloat(hDlg, IDC_SLIDER_CAM_DIST, 5.0f, 25.0f);
  cfg.cameraHeight =
      GetSliderValueFloat(hDlg, IDC_SLIDER_CAM_HEIGHT, 0.0f, 15.0f);
  cfg.fieldOfView = (float)GetSliderValue(hDlg, IDC_SLIDER_FOV);

  // Background
  cfg.skyboxEnabled =
      (IsDlgButtonChecked(hDlg, IDC_CHECK_SKYBOX) == BST_CHECKED);
  cfg.bgColorR = GetSliderValue(hDlg, IDC_SLIDER_BG_R);
  cfg.bgColorG = GetSliderValue(hDlg, IDC_SLIDER_BG_G);
  cfg.bgColorB = GetSliderValue(hDlg, IDC_SLIDER_BG_B);

  // Metrics
  ReadMetricControls(hDlg, cfg.cpuMetric, IDC_CHECK_CPU_ENABLED,
                     IDC_SLIDER_CPU_THRESHOLD, IDC_SLIDER_CPU_STRENGTH,
                     IDC_COMBO_CPU_MESH);
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

// Update slider label when value changes
static void UpdateSliderLabel(HWND hDlg, int sliderId, int labelId,
                              bool isFloat, float minVal = 0,
                              float maxVal = 100, int steps = 100) {
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

// Dialog procedure
static INT_PTR CALLBACK SettingsDialogProc(HWND hDlg, UINT message,
                                           WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    g_tempConfig = *g_pConfig;
    InitDialogFromConfig(hDlg, g_tempConfig);
    return TRUE;

  case WM_HSCROLL: {
    HWND hSlider = (HWND)lParam;
    int id = GetDlgCtrlID(hSlider);

    // Map sliders to their labels and float params
    struct SliderInfo {
      int labelId;
      bool isFloat;
      float minVal;
      float maxVal;
    };
    SliderInfo info = {0, false, 0, 100};

    switch (id) {
    case IDC_SLIDER_BLOOM_THRESH:
      info = {IDC_STATIC_BLOOM_THRESH, true, 0.3f, 1.5f};
      break;
    case IDC_SLIDER_BLOOM_STR:
      info = {IDC_STATIC_BLOOM_STR, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_EXPOSURE:
      info = {IDC_STATIC_EXPOSURE, true, 0.5f, 3.0f};
      break;
    case IDC_SLIDER_FOG_DENSITY:
      info = {IDC_STATIC_FOG_DENSITY, true, 0.0f, 0.2f};
      break;
    case IDC_SLIDER_PARTICLE_CNT:
      info = {IDC_STATIC_PARTICLE_CNT, false, 0, 0};
      break;
    case IDC_SLIDER_ROTATION:
      info = {IDC_STATIC_ROTATION, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_CAM_DIST:
      info = {IDC_STATIC_CAM_DIST, true, 5.0f, 25.0f};
      break;
    case IDC_SLIDER_CAM_HEIGHT:
      info = {IDC_STATIC_CAM_HEIGHT, true, 0.0f, 15.0f};
      break;
    case IDC_SLIDER_FOV:
      info = {IDC_STATIC_FOV, false, 0, 0};
      break;
    case IDC_SLIDER_BG_R:
      info = {IDC_STATIC_BG_R, false, 0, 0};
      break;
    case IDC_SLIDER_BG_G:
      info = {IDC_STATIC_BG_G, false, 0, 0};
      break;
    case IDC_SLIDER_BG_B:
      info = {IDC_STATIC_BG_B, false, 0, 0};
      break;
    // Metric sliders
    case IDC_SLIDER_CPU_THRESHOLD:
      info = {IDC_STATIC_CPU_THRESHOLD, true, 0.0f, 100.0f};
      break;
    case IDC_SLIDER_CPU_STRENGTH:
      info = {IDC_STATIC_CPU_STRENGTH, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_RAM_THRESHOLD:
      info = {IDC_STATIC_RAM_THRESHOLD, true, 0.0f, 100.0f};
      break;
    case IDC_SLIDER_RAM_STRENGTH:
      info = {IDC_STATIC_RAM_STRENGTH, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_DISK_THRESHOLD:
      info = {IDC_STATIC_DISK_THRESHOLD, true, 0.0f, 100.0f};
      break;
    case IDC_SLIDER_DISK_STRENGTH:
      info = {IDC_STATIC_DISK_STRENGTH, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_NET_THRESHOLD:
      info = {IDC_STATIC_NET_THRESHOLD, true, 0.0f, 100.0f};
      break;
    case IDC_SLIDER_NET_STRENGTH:
      info = {IDC_STATIC_NET_STRENGTH, true, 0.0f, 2.0f};
      break;
    // Layer Layout sliders
    case IDC_SLIDER_LAYER_X:
      info = {IDC_STATIC_LAYER_X, true, 0.0f, 1.0f};
      break;
    case IDC_SLIDER_LAYER_Y:
      info = {IDC_STATIC_LAYER_Y, true, 0.0f, 1.0f};
      break;
    case IDC_SLIDER_LAYER_W:
      info = {IDC_STATIC_LAYER_W, true, 0.0f, 1.0f};
      break;
    case IDC_SLIDER_LAYER_H:
      info = {IDC_STATIC_LAYER_H, true, 0.0f, 1.0f};
      break;
    case IDC_SLIDER_LAYER_DEPTH:
      info = {IDC_STATIC_LAYER_DEPTH, true, -1.0f, 1.0f};
      break;
    case IDC_SLIDER_LAYER_ROTATION:
      info = {IDC_STATIC_LAYER_ROTATION, true, 0.0f, 360.0f};
      break;
    // Layer FX sliders
    case IDC_SLIDER_FX_BLOOM_THRESH:
      info = {IDC_STATIC_FX_BLOOM_THRESH, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_FX_BLOOM_INT:
      info = {IDC_STATIC_FX_BLOOM_INT, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_FX_BLOOM_RAD:
      info = {IDC_STATIC_FX_BLOOM_RAD, true, 1.0f, 20.0f};
      break;
    case IDC_SLIDER_FX_GLOW_INT:
      info = {IDC_STATIC_FX_GLOW_INT, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_FX_GLOW_SIZE:
      info = {IDC_STATIC_FX_GLOW_SIZE, true, 1.0f, 10.0f};
      break;
    case IDC_SLIDER_FX_CHROM_OFFSET:
      info = {IDC_STATIC_FX_CHROM_OFFSET, true, 0.0f, 10.0f};
      break;
    case IDC_SLIDER_FX_DISTORT_AMT:
      info = {IDC_STATIC_FX_DISTORT_AMT, true, 0.0f, 1.0f};
      break;
    case IDC_SLIDER_FX_TRAIL_FADE:
      info = {IDC_STATIC_FX_TRAIL_FADE, true, 0.0f, 1.0f};
      break;
    case IDC_SLIDER_FX_OPACITY:
      info = {IDC_STATIC_FX_OPACITY, true, 0.0f, 1.0f};
      break;
    }

    if (info.labelId != 0) {
      UpdateSliderLabel(hDlg, id, info.labelId, info.isFloat, info.minVal,
                        info.maxVal);
    }
    return TRUE;
  }

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      ReadConfigFromDialog(hDlg, *g_pConfig);
      EndDialog(hDlg, IDOK);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      return TRUE;

    case IDC_BTN_DEFAULTS:
      g_tempConfig = Config();
      // Restore default presets logic similar to Engine init if desired
      // For now just standard defaults
      InitDialogFromConfig(hDlg, g_tempConfig);
      return TRUE;

    // Handle Layer Selection Change
    case IDC_COMBO_LAYER_SELECT:
      if (HIWORD(wParam) == CBN_SELCHANGE) {
        // Save current layer
        SaveLayerControls(hDlg, g_currentLayoutLayer);
        // Switch to new layer
        int newIndex = (int)SendMessage(
            GetDlgItem(hDlg, IDC_COMBO_LAYER_SELECT), CB_GETCURSEL, 0, 0);
        if (newIndex >= 0 && newIndex < 4) {
          g_currentLayoutLayer = newIndex;
          LoadLayerControls(hDlg, g_currentLayoutLayer);
        }
        return TRUE;
      }
      break;

    // Handle FX Layer Selection Change
    case IDC_COMBO_FX_LAYER_SELECT:
      if (HIWORD(wParam) == CBN_SELCHANGE) {
        // Save current layer
        SaveFXControls(hDlg, g_currentFXLayer);
        // Switch to new layer
        int newIndex = (int)SendMessage(
            GetDlgItem(hDlg, IDC_COMBO_FX_LAYER_SELECT), CB_GETCURSEL, 0, 0);
        if (newIndex >= 0 && newIndex < 4) {
          g_currentFXLayer = newIndex;
          LoadFXControls(hDlg, g_currentFXLayer);
        }
        return TRUE;
      }
      break;

    // Handle Layout Preset Change
    case IDC_COMBO_LAYOUT_PRESET:
      if (HIWORD(wParam) == CBN_SELCHANGE) {
        int presetIndex = (int)SendMessage(
            GetDlgItem(hDlg, IDC_COMBO_LAYOUT_PRESET), CB_GETCURSEL, 0, 0);
        LayerTransform t;

        // 0=Full, 1=TopLeft, 2=TopRight, 3=BotLeft, 4=BotRight, 5=Quad
        switch (presetIndex) {
        case 0:
          t = LayerTransform::FullScreen();
          break;
        case 1:
          t = LayerTransform::TopLeft();
          break;
        case 2:
          t = LayerTransform::TopRight();
          break;
        case 3:
          t = LayerTransform::BottomLeft();
          break;
        case 4:
          t = LayerTransform::BottomRight();
          break;
        case 5:
          // Quad layout logic depends on layer index, but here we just picking
          // a generic quad slot? Or maybe we should apply the "Quad Layout"
          // preset to ALL layers? "Quad Layout" implies configuring the current
          // layer to be one of the quads. Let's assume it sets it to a generic
          // quad size (0.5, 0.5) and let user position it? Or better, set it
          // based on current layer index.
          t = LayerTransform::QuadLayout(g_currentLayoutLayer);
          break;
        default:
          return TRUE;
        }

        // Apply to current temp config
        g_tempConfig.layerConfigs[g_currentLayoutLayer].transform = t;
        // Reload controls
        LoadLayerControls(hDlg, g_currentLayoutLayer);
        return TRUE;
      }
      break;
    }
    break;
  }
  return FALSE;
}

// Create dialog template in memory (no .rc file needed)
static LPCDLGTEMPLATE CreateDialogTemplate() {
  static BYTE buffer[16384] = {0}; // Increased buffer size
  LPWORD p = (LPWORD)buffer;

  // Dialog header
  LPDLGTEMPLATE pDlg = (LPDLGTEMPLATE)p;
  pDlg->style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU |
                WS_VISIBLE;
  pDlg->dwExtendedStyle = 0;
  pDlg->cdit = 0;
  pDlg->x = 0;
  pDlg->y = 0;
  pDlg->cx = 400; // Wider dialog
  pDlg->cy = 550; // Taller dialog

  p = (LPWORD)(pDlg + 1);
  *p++ = 0; // No menu
  *p++ = 0; // Default class

  const wchar_t *title = L"Screensaver Settings";
  wcscpy_s((wchar_t *)p, 64, title);
  p += wcslen(title) + 1;

  if (((ULONG_PTR)p) % 4)
    p++;

  // Helper lambda to add items
  auto AddItem = [&](DWORD style, short x, short y, short cx, short cy, WORD id,
                     const wchar_t *cls, const wchar_t *text) {
    if (((ULONG_PTR)p) % 4)
      p++;
    LPDLGITEMTEMPLATE pItem = (LPDLGITEMTEMPLATE)p;
    pItem->style = style | WS_CHILD | WS_VISIBLE;
    pItem->dwExtendedStyle = 0;
    pItem->x = x;
    pItem->y = y;
    pItem->cx = cx;
    pItem->cy = cy;
    pItem->id = id;
    p = (LPWORD)(pItem + 1);

    if (wcscmp(cls, L"STATIC") == 0) {
      *p++ = 0xFFFF;
      *p++ = 0x0082;
    } else if (wcscmp(cls, L"BUTTON") == 0) {
      *p++ = 0xFFFF;
      *p++ = 0x0080;
    } else if (wcscmp(cls, L"EDIT") == 0) {
      *p++ = 0xFFFF;
      *p++ = 0x0081;
    } else if (wcscmp(cls, L"COMBOBOX") == 0) {
      *p++ = 0xFFFF;
      *p++ = 0x0085;
    } else {
      wcscpy_s((wchar_t *)p, 64, cls);
      p += wcslen(cls) + 1;
    }

    if (text) {
      wcscpy_s((wchar_t *)p, 128, text);
      p += wcslen(text) + 1;
    } else {
      *p++ = 0;
    }

    *p++ = 0;
    pDlg->cdit++;
  };

  int y = 8;
  int labelW = 100;
  int sliderW = 130;
  int valueW = 40;
  int rowH = 16;

  // Quality section
  AddItem(SS_LEFT, 10, y, labelW, 12, IDC_LABEL_QUALITY, L"STATIC",
          L"Quality:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 115, y - 2, 80, 100, IDC_COMBO_QUALITY,
          L"COMBOBOX", nullptr);
  y += rowH + 4;

  // -- Visual Effects --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Visual Effects");
  y += rowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 60, 12, IDC_CHECK_BLOOM,
          L"BUTTON", L"Bloom");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 80, y, 60, 12, IDC_CHECK_FXAA,
          L"BUTTON", L"FXAA");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_BLOOM_THRESH, L"STATIC",
          L"Bloom Thresh:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_BLOOM_THRESH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_BLOOM_THRESH, L"STATIC",
          L"0.70");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_BLOOM_STR, L"STATIC",
          L"Bloom Strength:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_BLOOM_STR, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_BLOOM_STR, L"STATIC",
          L"0.80");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_EXPOSURE, L"STATIC", L"Exposure:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_EXPOSURE, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_EXPOSURE, L"STATIC", L"1.00");
  y += rowH + 4;

  // -- Fog --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 10, y, 80, 12, IDC_CHECK_FOG, L"BUTTON",
          L"Fog");
  AddItem(SS_LEFT, 100, y, 60, 12, IDC_LABEL_FOG_DENSITY, L"STATIC",
          L"Density:");
  AddItem(0, 160, y, 80, 14, IDC_SLIDER_FOG_DENSITY, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_FOG_DENSITY, L"STATIC",
          L"0.05");
  y += rowH + 4;

  // -- Particles --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 10, y, 80, 12, IDC_CHECK_PARTICLES,
          L"BUTTON", L"Particles");
  AddItem(SS_LEFT, 100, y, 50, 12, IDC_LABEL_PARTICLE_CNT, L"STATIC",
          L"Count:");
  AddItem(0, 150, y, 90, 14, IDC_SLIDER_PARTICLE_CNT, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_PARTICLE_CNT, L"STATIC",
          L"6000");
  y += rowH + 4;

  // -- Camera & Scene --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Camera & Scene");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_ROTATION, L"STATIC",
          L"Rotation Speed:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_ROTATION, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_ROTATION, L"STATIC", L"0.20");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_CAM_DIST, L"STATIC", L"Distance:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_CAM_DIST, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_CAM_DIST, L"STATIC",
          L"12.00");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_CAM_HEIGHT, L"STATIC", L"Height:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_CAM_HEIGHT, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_CAM_HEIGHT, L"STATIC",
          L"5.00");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 90, 12, IDC_LABEL_FOV, L"STATIC", L"Field of View:");
  AddItem(0, 110, y, sliderW, 14, IDC_SLIDER_FOV, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 245, y, valueW, 12, IDC_STATIC_FOV, L"STATIC", L"45");
  y += rowH + 4;

  // -- Background --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 10, y, 80, 12, IDC_CHECK_SKYBOX,
          L"BUTTON", L"Skybox");
  AddItem(SS_LEFT, 100, y, 20, 12, IDC_LABEL_BG_R, L"STATIC", L"R:");
  AddItem(0, 120, y, 50, 14, IDC_SLIDER_BG_R, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 172, y, 20, 12, IDC_STATIC_BG_R, L"STATIC", L"0");
  AddItem(SS_LEFT, 195, y, 20, 12, IDC_LABEL_BG_G, L"STATIC", L"G:");
  AddItem(0, 215, y, 50, 14, IDC_SLIDER_BG_G, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 267, y, 20, 12, IDC_STATIC_BG_G, L"STATIC", L"13");
  AddItem(SS_LEFT, 290, y, 20, 12, IDC_LABEL_BG_B, L"STATIC", L"B:");
  AddItem(0, 310, y, 50, 14, IDC_SLIDER_BG_B, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 362, y, 20, 12, IDC_STATIC_BG_B, L"STATIC", L"25");
  y += rowH + 6;

  // =====================================================================
  // SYSTEM METRICS SECTION
  // =====================================================================
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(SS_LEFT, 10, y, 150, 12, 0, L"STATIC",
          L"System Metrics Visualization");
  y += rowH;

  // Inline metric sections (helper removed for C++17 compat)
  int mSliderW = 100;
  int mValueW = 35;
  int mRowH = 16;

  // CPU Metric
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 70, 12, IDC_CHECK_CPU_ENABLED,
          L"BUTTON", L"CPU");
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_CPU_THRESHOLD, L"STATIC",
          L"Thresh:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_CPU_THRESHOLD, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_CPU_THRESHOLD, L"STATIC",
          L"0");
  AddItem(SS_LEFT, 280, y, 30, 12, IDC_LABEL_CPU_MESH, L"STATIC", L"Mesh:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 310, y - 2, 70, 100,
          IDC_COMBO_CPU_MESH, L"COMBOBOX", nullptr);
  y += mRowH;
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_CPU_STRENGTH, L"STATIC",
          L"Strength:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_CPU_STRENGTH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_CPU_STRENGTH, L"STATIC",
          L"1.00");
  y += mRowH + 2;

  // RAM Metric
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 70, 12, IDC_CHECK_RAM_ENABLED,
          L"BUTTON", L"RAM");
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_RAM_THRESHOLD, L"STATIC",
          L"Thresh:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_RAM_THRESHOLD, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_RAM_THRESHOLD, L"STATIC",
          L"0");
  AddItem(SS_LEFT, 280, y, 30, 12, IDC_LABEL_RAM_MESH, L"STATIC", L"Mesh:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 310, y - 2, 70, 100,
          IDC_COMBO_RAM_MESH, L"COMBOBOX", nullptr);
  y += mRowH;
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_RAM_STRENGTH, L"STATIC",
          L"Strength:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_RAM_STRENGTH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_RAM_STRENGTH, L"STATIC",
          L"1.00");
  y += mRowH + 2;

  // Disk Metric
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 70, 12, IDC_CHECK_DISK_ENABLED,
          L"BUTTON", L"Disk");
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_DISK_THRESHOLD, L"STATIC",
          L"Thresh:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_DISK_THRESHOLD, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_DISK_THRESHOLD, L"STATIC",
          L"0");
  AddItem(SS_LEFT, 280, y, 30, 12, IDC_LABEL_DISK_MESH, L"STATIC", L"Mesh:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 310, y - 2, 70, 100,
          IDC_COMBO_DISK_MESH, L"COMBOBOX", nullptr);
  y += mRowH;
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_DISK_STRENGTH, L"STATIC",
          L"Strength:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_DISK_STRENGTH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_DISK_STRENGTH, L"STATIC",
          L"1.00");
  y += mRowH + 2;

  // Network Metric
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 70, 12, IDC_CHECK_NET_ENABLED,
          L"BUTTON", L"Network");
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_NET_THRESHOLD, L"STATIC",
          L"Thresh:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_NET_THRESHOLD, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_NET_THRESHOLD, L"STATIC",
          L"0");
  AddItem(SS_LEFT, 280, y, 30, 12, IDC_LABEL_NET_MESH, L"STATIC", L"Mesh:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 310, y - 2, 70, 100,
          IDC_COMBO_NET_MESH, L"COMBOBOX", nullptr);
  y += mRowH;
  AddItem(SS_LEFT, 90, y, 50, 12, IDC_LABEL_NET_STRENGTH, L"STATIC",
          L"Strength:");
  AddItem(0, 140, y, mSliderW, 14, IDC_SLIDER_NET_STRENGTH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 245, y, mValueW, 12, IDC_STATIC_NET_STRENGTH, L"STATIC",
          L"1.00");
  y += mRowH + 2;

  y += 6;

  // -- Layer Layout --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Layer Layout");
  AddItem(SS_LEFT, 120, y, 40, 12, 0, L"STATIC", L"Layer:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 160, y - 2, 70, 100,
          IDC_COMBO_LAYER_SELECT, L"COMBOBOX", nullptr);
  AddItem(SS_LEFT, 240, y, 40, 12, 0, L"STATIC", L"Preset:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 280, y - 2, 90, 100,
          IDC_COMBO_LAYOUT_PRESET, L"COMBOBOX", nullptr);
  y += mRowH;

  AddItem(SS_LEFT, 15, y, 30, 12, IDC_LABEL_LAYER_X, L"STATIC", L"X:");
  AddItem(0, 45, y, 60, 14, IDC_SLIDER_LAYER_X, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 108, y, 30, 12, IDC_STATIC_LAYER_X, L"STATIC", L"0.00");
  AddItem(SS_LEFT, 145, y, 30, 12, IDC_LABEL_LAYER_Y, L"STATIC", L"Y:");
  AddItem(0, 175, y, 60, 14, IDC_SLIDER_LAYER_Y, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 238, y, 30, 12, IDC_STATIC_LAYER_Y, L"STATIC", L"0.00");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 280, y, 80, 12, IDC_CHECK_LAYER_VISIBLE,
          L"BUTTON", L"Visible");
  y += mRowH;

  AddItem(SS_LEFT, 15, y, 30, 12, IDC_LABEL_LAYER_W, L"STATIC", L"W:");
  AddItem(0, 45, y, 60, 14, IDC_SLIDER_LAYER_W, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 108, y, 30, 12, IDC_STATIC_LAYER_W, L"STATIC", L"1.00");
  AddItem(SS_LEFT, 145, y, 30, 12, IDC_LABEL_LAYER_H, L"STATIC", L"H:");
  AddItem(0, 175, y, 60, 14, IDC_SLIDER_LAYER_H, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 238, y, 30, 12, IDC_STATIC_LAYER_H, L"STATIC", L"1.00");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 280, y, 80, 12,
          IDC_CHECK_LAYER_LOCK_ASPECT, L"BUTTON", L"Lock Aspect");
  y += mRowH + 2;

  // -- Layer Effects --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 375, 1, 0, L"STATIC", nullptr);
  y += 4;
  AddItem(SS_LEFT, 10, y, 80, 12, 0, L"STATIC", L"Layer Effects");
  AddItem(SS_LEFT, 100, y, 40, 12, 0, L"STATIC", L"Layer:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 140, y - 2, 70, 100,
          IDC_COMBO_FX_LAYER_SELECT, L"COMBOBOX", nullptr);
  AddItem(SS_LEFT, 220, y, 40, 12, 0, L"STATIC", L"Blend:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 260, y - 2, 70, 100,
          IDC_COMBO_BLEND_MODE, L"COMBOBOX", nullptr);
  y += mRowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 55, 12, IDC_CHECK_FX_BLOOM,
          L"BUTTON", L"Bloom");
  AddItem(0, 75, y, 60, 14, IDC_SLIDER_FX_BLOOM_INT, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 138, y, 30, 12, IDC_STATIC_FX_BLOOM_INT, L"STATIC", L"0.80");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 180, y, 55, 12, IDC_CHECK_FX_GLOW,
          L"BUTTON", L"Glow");
  AddItem(0, 235, y, 60, 14, IDC_SLIDER_FX_GLOW_INT, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 298, y, 30, 12, IDC_STATIC_FX_GLOW_INT, L"STATIC", L"0.50");
  y += mRowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 70, 12, IDC_CHECK_FX_CHROM,
          L"BUTTON", L"Chromatic");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 95, y, 65, 12, IDC_CHECK_FX_DISTORT,
          L"BUTTON", L"Distortion");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 170, y, 50, 12, IDC_CHECK_FX_TRAILS,
          L"BUTTON", L"Trails");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 225, y, 70, 12, IDC_CHECK_FX_SCANLINES,
          L"BUTTON", L"Scanlines");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 300, y, 50, 12, IDC_CHECK_FX_NOISE,
          L"BUTTON", L"Noise");
  y += mRowH;

  AddItem(SS_LEFT, 15, y, 45, 12, 0, L"STATIC", L"Opacity:");
  AddItem(0, 65, y, 80, 14, IDC_SLIDER_FX_OPACITY, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 150, y, 30, 12, IDC_STATIC_FX_OPACITY, L"STATIC", L"1.00");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 200, y, 75, 12, IDC_CHECK_FX_MOTIONBLUR,
          L"BUTTON", L"Motion Blur");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 285, y, 60, 12, IDC_CHECK_FX_PIXELATE,
          L"BUTTON", L"Pixelate");
  y += mRowH + 4;

  // Buttons
  AddItem(BS_DEFPUSHBUTTON | WS_TABSTOP, 100, y, 60, 18, IDOK, L"BUTTON",
          L"OK");
  AddItem(BS_PUSHBUTTON | WS_TABSTOP, 165, y, 60, 18, IDCANCEL, L"BUTTON",
          L"Cancel");
  AddItem(BS_PUSHBUTTON | WS_TABSTOP, 230, y, 60, 18, IDC_BTN_DEFAULTS,
          L"BUTTON", L"Defaults");

  pDlg->cy = y + 28;

  return (LPCDLGTEMPLATE)buffer;
}

bool ShowSettingsDialog(HINSTANCE hInstance, HWND parent, Config &config) {
  INITCOMMONCONTROLSEX icex;
  icex.dwSize = sizeof(icex);
  icex.dwICC = ICC_BAR_CLASSES;
  InitCommonControlsEx(&icex);

  g_pConfig = &config;

  LPCDLGTEMPLATE pTemplate = CreateDialogTemplate();
  INT_PTR result =
      DialogBoxIndirectW(hInstance, pTemplate, parent, SettingsDialogProc);

  return (result == IDOK);
}
