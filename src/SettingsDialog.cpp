#include "SettingsDialog.h"
#include "resource.h"
#include <commctrl.h>
#include <cstdio>
#include <string>
#include <vector>


#pragma comment(lib, "comctl32.lib")

// Global config pointer for dialog proc
static Config *g_pConfig = nullptr;
static Config g_tempConfig;
static int g_currentLayoutLayer = 0;
static int g_currentFXLayer = 0;

// Tab Constants
enum SettingsTab {
  TAB_GENERAL = 0,
  TAB_VISUALS,
  TAB_METRICS,
  TAB_LAYOUT,
  TAB_LAYER_FX,
  TAB_COUNT
};

static HWND g_hTabControl = nullptr;
static int g_currentTab = TAB_GENERAL;

// Helper: Set slider value and update label
static void SetSliderValue(HWND hDlg, int sliderId, int labelId, int value,
                           int minVal, int maxVal) {
  HWND hSlider = GetDlgItem(hDlg, sliderId);
  SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
  SendMessage(hSlider, TBM_SETPOS, TRUE, value);
  ShowWindow(hSlider, SW_SHOW); // Ensure visible when setting value

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
  ShowWindow(hSlider, SW_SHOW);

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
  SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Sphere");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Cube");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Ring");
  SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"None");
  SendMessage(hCombo, CB_SETCURSEL, (int)selected, 0);
}

// Control Visibility Helper
static void ShowControlGroup(HWND hDlg, const std::vector<int> &cxIds,
                             bool show) {
  int cmd = show ? SW_SHOW : SW_HIDE;
  for (int id : cxIds) {
    ShowWindow(GetDlgItem(hDlg, id), cmd);
  }
}

// Group IDs for Tabs
static const std::vector<int> GROUP_GENERAL = {
    IDC_LABEL_QUALITY,   IDC_COMBO_QUALITY,     IDC_SLIDER_ROTATION,
    IDC_STATIC_ROTATION, IDC_CHECK_SKYBOX,      IDC_SLIDER_BG_R,
    IDC_STATIC_BG_R,     IDC_SLIDER_BG_G,       IDC_STATIC_BG_G,
    IDC_SLIDER_BG_B,     IDC_STATIC_BG_B,       IDC_SLIDER_CAM_DIST,
    IDC_STATIC_CAM_DIST, IDC_SLIDER_CAM_HEIGHT, IDC_STATIC_CAM_HEIGHT,
    IDC_SLIDER_FOV,      IDC_STATIC_FOV};

static const std::vector<int> GROUP_VISUALS = {
    IDC_CHECK_BLOOM,         IDC_CHECK_FXAA,          IDC_LABEL_BLOOM_THRESH,
    IDC_SLIDER_BLOOM_THRESH, IDC_STATIC_BLOOM_THRESH, IDC_LABEL_BLOOM_STR,
    IDC_SLIDER_BLOOM_STR,    IDC_STATIC_BLOOM_STR,    IDC_LABEL_EXPOSURE,
    IDC_SLIDER_EXPOSURE,     IDC_STATIC_EXPOSURE,     IDC_CHECK_FOG,
    IDC_LABEL_FOG_DENSITY,   IDC_SLIDER_FOG_DENSITY,  IDC_STATIC_FOG_DENSITY,
    IDC_CHECK_PARTICLES,     IDC_LABEL_PARTICLE_CNT,  IDC_SLIDER_PARTICLE_CNT,
    IDC_STATIC_PARTICLE_CNT};

static const std::vector<int> GROUP_METRICS = {
    IDC_CHECK_CPU_ENABLED,     IDC_SLIDER_CPU_THRESHOLD,
    IDC_STATIC_CPU_THRESHOLD,  IDC_SLIDER_CPU_STRENGTH,
    IDC_STATIC_CPU_STRENGTH,   IDC_COMBO_CPU_MESH,
    IDC_CHECK_RAM_ENABLED,     IDC_SLIDER_RAM_THRESHOLD,
    IDC_STATIC_RAM_THRESHOLD,  IDC_SLIDER_RAM_STRENGTH,
    IDC_STATIC_RAM_STRENGTH,   IDC_COMBO_RAM_MESH,
    IDC_CHECK_DISK_ENABLED,    IDC_SLIDER_DISK_THRESHOLD,
    IDC_STATIC_DISK_THRESHOLD, IDC_SLIDER_DISK_STRENGTH,
    IDC_STATIC_DISK_STRENGTH,  IDC_COMBO_DISK_MESH,
    IDC_CHECK_NET_ENABLED,     IDC_SLIDER_NET_THRESHOLD,
    IDC_STATIC_NET_THRESHOLD,  IDC_SLIDER_NET_STRENGTH,
    IDC_STATIC_NET_STRENGTH,   IDC_COMBO_NET_MESH};

static const std::vector<int> GROUP_LAYOUT = {
    IDC_COMBO_LAYER_SELECT,  IDC_COMBO_LAYOUT_PRESET,
    IDC_SLIDER_LAYER_X,      IDC_STATIC_LAYER_X,
    IDC_SLIDER_LAYER_Y,      IDC_STATIC_LAYER_Y,
    IDC_SLIDER_LAYER_W,      IDC_STATIC_LAYER_W,
    IDC_SLIDER_LAYER_H,      IDC_STATIC_LAYER_H,
    IDC_CHECK_LAYER_VISIBLE, IDC_CHECK_LAYER_LOCK_ASPECT};

static const std::vector<int> GROUP_LAYER_FX = {
    IDC_COMBO_FX_LAYER_SELECT,  IDC_COMBO_BLEND_MODE,
    IDC_SLIDER_FX_OPACITY,      IDC_STATIC_FX_OPACITY,
    IDC_CHECK_FX_BLOOM,         IDC_SLIDER_FX_BLOOM_INT,
    IDC_STATIC_FX_BLOOM_INT,    IDC_SLIDER_FX_BLOOM_THRESH,
    IDC_STATIC_FX_BLOOM_THRESH, IDC_SLIDER_FX_BLOOM_RAD,
    IDC_STATIC_FX_BLOOM_RAD,    IDC_CHECK_FX_GLOW,
    IDC_SLIDER_FX_GLOW_INT,     IDC_STATIC_FX_GLOW_INT,
    IDC_SLIDER_FX_GLOW_SIZE,    IDC_STATIC_FX_GLOW_SIZE,
    IDC_CHECK_FX_CHROM,         IDC_SLIDER_FX_CHROM_OFFSET,
    IDC_STATIC_FX_CHROM_OFFSET, IDC_CHECK_FX_DISTORT,
    IDC_SLIDER_FX_DISTORT_AMT,  IDC_STATIC_FX_DISTORT_AMT,
    IDC_CHECK_FX_TRAILS,        IDC_SLIDER_FX_TRAIL_FADE,
    IDC_STATIC_FX_TRAIL_FADE,   IDC_CHECK_FX_SCANLINES,
    IDC_CHECK_FX_NOISE,         IDC_CHECK_FX_MOTIONBLUR,
    IDC_CHECK_FX_PIXELATE};

static void UpdateTabVisibility(HWND hDlg) {
  ShowControlGroup(hDlg, GROUP_GENERAL, g_currentTab == TAB_GENERAL);
  ShowControlGroup(hDlg, GROUP_VISUALS, g_currentTab == TAB_VISUALS);
  ShowControlGroup(hDlg, GROUP_METRICS, g_currentTab == TAB_METRICS);
  ShowControlGroup(hDlg, GROUP_LAYOUT, g_currentTab == TAB_LAYOUT);
  ShowControlGroup(hDlg, GROUP_LAYER_FX, g_currentTab == TAB_LAYER_FX);
}

// Helpers for groups
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

static void ReadMetricControls(HWND hDlg, MetricConfig &metric, int checkId,
                               int threshSliderId, int strSliderId,
                               int meshComboId) {
  metric.enabled = (IsDlgButtonChecked(hDlg, checkId) == BST_CHECKED);
  metric.threshold = GetSliderValueFloat(hDlg, threshSliderId, 0.0f, 100.0f);
  metric.strength = GetSliderValueFloat(hDlg, strSliderId, 0.0f, 2.0f);
  metric.meshType =
      (MeshType)SendMessage(GetDlgItem(hDlg, meshComboId), CB_GETCURSEL, 0, 0);
}

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

static void InitDialogFromConfig(HWND hDlg, const Config &cfg) {
  // Quality
  HWND hQuality = GetDlgItem(hDlg, IDC_COMBO_QUALITY);
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"Low");
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"Medium");
  SendMessageW(hQuality, CB_ADDSTRING, 0, (LPARAM)L"High");
  SendMessage(hQuality, CB_SETCURSEL, (int)cfg.quality, 0);

  // General controls
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

  // Visuals
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

  // Layout Layer Combo
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

  g_currentLayoutLayer = 0;
  LoadLayerControls(hDlg, 0);

  // Layer FX Combo
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

  g_currentFXLayer = 0;
  LoadFXControls(hDlg, 0);

  // Refresh Visibility
  UpdateTabVisibility(hDlg);
}

static void ReadConfigFromDialog(HWND hDlg, Config &cfg) {
  SaveLayerControls(hDlg, g_currentLayoutLayer);
  SaveFXControls(hDlg, g_currentFXLayer);
  cfg.layerConfigs = g_tempConfig.layerConfigs;

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
  case WM_INITDIALOG: {
    g_hTabControl = GetDlgItem(hDlg, IDC_TAB_CONTROL);

    TCITEMW tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)L"General";
    SendMessage(g_hTabControl, TCM_INSERTITEM, 0, (LPARAM)&tie);
    tie.pszText = (LPWSTR)L"Visuals";
    SendMessage(g_hTabControl, TCM_INSERTITEM, 1, (LPARAM)&tie);
    tie.pszText = (LPWSTR)L"Metrics";
    SendMessage(g_hTabControl, TCM_INSERTITEM, 2, (LPARAM)&tie);
    tie.pszText = (LPWSTR)L"Layout";
    SendMessage(g_hTabControl, TCM_INSERTITEM, 3, (LPARAM)&tie);
    tie.pszText = (LPWSTR)L"Layer FX";
    SendMessage(g_hTabControl, TCM_INSERTITEM, 4, (LPARAM)&tie);

    g_tempConfig = *g_pConfig;
    InitDialogFromConfig(hDlg, g_tempConfig);
    return TRUE;
  }

  case WM_NOTIFY: {
    LPNMHDR pHdr = (LPNMHDR)lParam;
    if (pHdr->idFrom == IDC_TAB_CONTROL && pHdr->code == TCN_SELCHANGE) {
      g_currentTab = (int)SendMessage(g_hTabControl, TCM_GETCURSEL, 0, 0);
      UpdateTabVisibility(hDlg);
      return TRUE;
    }
    break;
  }

  case WM_HSCROLL: {
    HWND hSlider = (HWND)lParam;
    int id = GetDlgCtrlID(hSlider);

    // Simple exhaustive switch for now, matching existing logic
    // (Collapsed for brevity, logic follows InitDialog calls)
    float minVal = 0, maxVal = 100;
    bool isFloat = false;
    int labelId = 0;

    // ... Mapping Logic ...
    // Note: Since I am replacing the file, I must reconstruct this mapping.
    // Quality of life: I will use a helper structure or just a huge switch.
    // The previous implementation used a switch with a struct. I'll stick to
    // that.

    struct SliderInfo {
      int labelId;
      bool isFloat;
      float min;
      float max;
    };
    SliderInfo info = {0, false, 0, 100};

    switch (id) {
    case IDC_SLIDER_ROTATION:
      info = {IDC_STATIC_ROTATION, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_BG_R:
      info = {IDC_STATIC_BG_R, false, 0, 255};
      break;
    case IDC_SLIDER_BG_G:
      info = {IDC_STATIC_BG_G, false, 0, 255};
      break;
    case IDC_SLIDER_BG_B:
      info = {IDC_STATIC_BG_B, false, 0, 255};
      break;
    case IDC_SLIDER_CAM_DIST:
      info = {IDC_STATIC_CAM_DIST, true, 5.0f, 25.0f};
      break;
    case IDC_SLIDER_CAM_HEIGHT:
      info = {IDC_STATIC_CAM_HEIGHT, true, 0.0f, 15.0f};
      break;
    case IDC_SLIDER_FOV:
      info = {IDC_STATIC_FOV, false, 30, 90};
      break;

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
      info = {IDC_STATIC_PARTICLE_CNT, false, 500, 10000};
      break;

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

    case IDC_SLIDER_FX_BLOOM_INT:
      info = {IDC_STATIC_FX_BLOOM_INT, true, 0.0f, 2.0f};
      break;
    case IDC_SLIDER_FX_BLOOM_THRESH:
      info = {IDC_STATIC_FX_BLOOM_THRESH, true, 0.0f, 2.0f};
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
      UpdateSliderLabel(hDlg, id, info.labelId, info.isFloat, info.min,
                        info.max);
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
      InitDialogFromConfig(hDlg, g_tempConfig);
      return TRUE;

    // Handle Layer Selection Change
    case IDC_COMBO_LAYER_SELECT:
      if (HIWORD(wParam) == CBN_SELCHANGE) {
        SaveLayerControls(hDlg, g_currentLayoutLayer);
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
        SaveFXControls(hDlg, g_currentFXLayer);
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
        switch (presetIndex) {
        case 0:
          t = LayerTransform::FullScreen();
          break; // Full
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
          t = LayerTransform::QuadLayout(g_currentLayoutLayer);
          break;
        default:
          return TRUE;
        }
        g_tempConfig.layerConfigs[g_currentLayoutLayer].transform = t;
        LoadLayerControls(hDlg, g_currentLayoutLayer);
        return TRUE;
      }
      break;
    }
    break;
  }
  return FALSE;
}

// Create dialog template in memory
static LPCDLGTEMPLATE CreateDialogTemplate() {
  static BYTE buffer[32768] = {0};
  LPWORD p = (LPWORD)buffer;

  // Dialog header
  LPDLGTEMPLATE pDlg = (LPDLGTEMPLATE)p;
  pDlg->style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU |
                WS_VISIBLE;
  pDlg->dwExtendedStyle = 0;
  pDlg->cdit = 0;
  pDlg->x = 0;
  pDlg->y = 0;
  pDlg->cx = 400;
  pDlg->cy = 300; // Smaller height!

  p = (LPWORD)(pDlg + 1);
  *p++ = 0; // No menu
  *p++ = 0; // Default class

  const wchar_t *title = L"Screensaver Settings";
  wcscpy_s((wchar_t *)p, 64, title);
  p += wcslen(title) + 1;

  if (((ULONG_PTR)p) % 4)
    p++;

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

  // Add TAB Control
  AddItem(WS_TABSTOP, 5, 5, 390, 260, IDC_TAB_CONTROL, WC_TABCONTROLW, nullptr);

  // Common Layout Params
  int x = 20, y = 35;
  int labelW = 90, sliderW = 180, valueW = 40, rowH = 18;

  // --- GENERAL TAB ---
  int startY = 35;
  y = startY;
  AddItem(SS_LEFT, x, y, labelW, 12, IDC_LABEL_QUALITY, L"STATIC", L"Quality:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, x + 100, y - 2, 80, 100,
          IDC_COMBO_QUALITY, L"COMBOBOX", nullptr);
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, 0, L"STATIC", L"Rotation Speed:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_ROTATION, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_ROTATION, L"STATIC",
          L"0.20");
  y += rowH;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_SKYBOX,
          L"BUTTON", L"Skybox");
  y += rowH;
  AddItem(SS_LEFT, x, y, 60, 12, 0, L"STATIC", L"BG Color:");
  AddItem(0, x + 60, y, 90, 14, IDC_SLIDER_BG_R, TRACKBAR_CLASSW, nullptr);
  AddItem(0, x + 160, y, 90, 14, IDC_SLIDER_BG_G, TRACKBAR_CLASSW, nullptr);
  AddItem(0, x + 260, y, 90, 14, IDC_SLIDER_BG_B, TRACKBAR_CLASSW, nullptr);
  // Add static labels for RGB if needed, but skipping for brevity
  y += rowH + 5;
  AddItem(SS_LEFT, x, y, labelW, 12, 0, L"STATIC", L"Cam Distance:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_CAM_DIST, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_CAM_DIST, L"STATIC",
          L"12.0");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, 0, L"STATIC", L"Cam Height:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_CAM_HEIGHT, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_CAM_HEIGHT, L"STATIC",
          L"5.0");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, 0, L"STATIC", L"FOV:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_FOV, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_FOV, L"STATIC", L"45");

  // --- VISUALS TAB ---
  y = startY;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_BLOOM,
          L"BUTTON", L"Bloom");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 100, y, 80, 12, IDC_CHECK_FXAA,
          L"BUTTON", L"FXAA");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, IDC_LABEL_BLOOM_THRESH, L"STATIC",
          L"Threshold:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_BLOOM_THRESH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_BLOOM_THRESH, L"STATIC",
          L"0.7");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, IDC_LABEL_BLOOM_STR, L"STATIC",
          L"Strength:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_BLOOM_STR, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_BLOOM_STR, L"STATIC",
          L"0.8");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, IDC_LABEL_EXPOSURE, L"STATIC",
          L"Exposure:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_EXPOSURE, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_EXPOSURE, L"STATIC",
          L"1.0");
  y += rowH + 5;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_FOG, L"BUTTON",
          L"Fog");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, IDC_LABEL_FOG_DENSITY, L"STATIC",
          L"Density:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_FOG_DENSITY, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_FOG_DENSITY, L"STATIC",
          L"0.05");
  y += rowH + 5;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_PARTICLES,
          L"BUTTON", L"Particles");
  y += rowH;
  AddItem(SS_LEFT, x, y, labelW, 12, IDC_LABEL_PARTICLE_CNT, L"STATIC",
          L"Count:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_PARTICLE_CNT, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_PARTICLE_CNT, L"STATIC",
          L"6000");

  // --- METRICS TAB ---
  // Condensed repeated controls for CPU, RAM, Disk, Net
  int my = startY;
  auto AddMetricRows = [&](int checkId, const wchar_t *title, int sliderT,
                           int statT, int sliderS, int statS, int comboId) {
    AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, my, 80, 12, checkId, L"BUTTON",
            title);
    AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, x + 150, my - 2, 80, 100, comboId,
            L"COMBOBOX", nullptr);
    my += rowH;
    AddItem(SS_LEFT, x + 20, my, 80, 12, 0, L"STATIC", L"Threshold:");
    AddItem(0, x + 100, my, 120, 14, sliderT, TRACKBAR_CLASSW, nullptr);
    AddItem(SS_LEFT, x + 230, my, 40, 12, statT, L"STATIC", L"0.0");
    my += rowH;
    AddItem(SS_LEFT, x + 20, my, 80, 12, 0, L"STATIC", L"Strength:");
    AddItem(0, x + 100, my, 120, 14, sliderS, TRACKBAR_CLASSW, nullptr);
    AddItem(SS_LEFT, x + 230, my, 40, 12, statS, L"STATIC", L"1.0");
    my += rowH + 4;
  };

  AddMetricRows(IDC_CHECK_CPU_ENABLED, L"CPU Usage", IDC_SLIDER_CPU_THRESHOLD,
                IDC_STATIC_CPU_THRESHOLD, IDC_SLIDER_CPU_STRENGTH,
                IDC_STATIC_CPU_STRENGTH, IDC_COMBO_CPU_MESH);
  AddMetricRows(IDC_CHECK_RAM_ENABLED, L"RAM Usage", IDC_SLIDER_RAM_THRESHOLD,
                IDC_STATIC_RAM_THRESHOLD, IDC_SLIDER_RAM_STRENGTH,
                IDC_STATIC_RAM_STRENGTH, IDC_COMBO_RAM_MESH);
  AddMetricRows(IDC_CHECK_DISK_ENABLED, L"Disk Usage",
                IDC_SLIDER_DISK_THRESHOLD, IDC_STATIC_DISK_THRESHOLD,
                IDC_SLIDER_DISK_STRENGTH, IDC_STATIC_DISK_STRENGTH,
                IDC_COMBO_DISK_MESH);
  AddMetricRows(IDC_CHECK_NET_ENABLED, L"Network", IDC_SLIDER_NET_THRESHOLD,
                IDC_STATIC_NET_THRESHOLD, IDC_SLIDER_NET_STRENGTH,
                IDC_STATIC_NET_STRENGTH, IDC_COMBO_NET_MESH);

  // --- LAYOUT TAB ---
  y = startY;
  AddItem(SS_LEFT, x, y, 60, 12, 0, L"STATIC", L"Layer:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, x + 70, y - 2, 80, 100,
          IDC_COMBO_LAYER_SELECT, L"COMBOBOX", nullptr);
  AddItem(SS_LEFT, x + 170, y, 60, 12, 0, L"STATIC", L"Preset:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, x + 230, y - 2, 80, 100,
          IDC_COMBO_LAYOUT_PRESET, L"COMBOBOX", nullptr);
  y += rowH * 2;

  auto AddLayoutSlider = [&](const wchar_t *lbl, int sid, int stid) {
    AddItem(SS_LEFT, x, y, 80, 12, 0, L"STATIC", lbl);
    AddItem(0, x + 90, y, 150, 14, sid, TRACKBAR_CLASSW, nullptr);
    AddItem(SS_LEFT, x + 250, y, valueW, 12, stid, L"STATIC", L"0.0");
    y += rowH;
  };
  AddLayoutSlider(L"Position X:", IDC_SLIDER_LAYER_X, IDC_STATIC_LAYER_X);
  AddLayoutSlider(L"Position Y:", IDC_SLIDER_LAYER_Y, IDC_STATIC_LAYER_Y);
  AddLayoutSlider(L"Width:", IDC_SLIDER_LAYER_W, IDC_STATIC_LAYER_W);
  AddLayoutSlider(L"Height:", IDC_SLIDER_LAYER_H, IDC_STATIC_LAYER_H);
  y += 5;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_LAYER_VISIBLE,
          L"BUTTON", L"Visible");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 100, y, 120, 12,
          IDC_CHECK_LAYER_LOCK_ASPECT, L"BUTTON", L"Lock Aspect");

  // --- LAYER FX TAB ---
  y = startY;
  AddItem(SS_LEFT, x, y, 60, 12, 0, L"STATIC", L"Layer:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, x + 70, y - 2, 80, 100,
          IDC_COMBO_FX_LAYER_SELECT, L"COMBOBOX", nullptr);
  AddItem(SS_LEFT, x + 160, y, 60, 12, 0, L"STATIC", L"Blend Mode:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, x + 230, y - 2, 80, 100,
          IDC_COMBO_BLEND_MODE, L"COMBOBOX", nullptr);
  y += rowH + 5;

  AddItem(SS_LEFT, x, y, labelW, 12, 0, L"STATIC", L"Opacity:");
  AddItem(0, x + 100, y, sliderW, 14, IDC_SLIDER_FX_OPACITY, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, x + 290, y, valueW, 12, IDC_STATIC_FX_OPACITY, L"STATIC",
          L"1.0");
  y += rowH + 5;

  // Condense FX checks
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_FX_BLOOM,
          L"BUTTON", L"Bloom");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 80, y, 80, 12, IDC_CHECK_FX_GLOW,
          L"BUTTON", L"Glow");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 160, y, 80, 12, IDC_CHECK_FX_CHROM,
          L"BUTTON", L"Abberation");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 240, y, 80, 12,
          IDC_CHECK_FX_DISTORT, L"BUTTON", L"Distortion");
  y += rowH;
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x, y, 80, 12, IDC_CHECK_FX_TRAILS,
          L"BUTTON", L"Trails");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 80, y, 80, 12,
          IDC_CHECK_FX_SCANLINES, L"BUTTON", L"Scan");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 160, y, 80, 12, IDC_CHECK_FX_NOISE,
          L"BUTTON", L"Noise");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, x + 240, y, 80, 12,
          IDC_CHECK_FX_PIXELATE, L"BUTTON", L"Pixelate");
  y += rowH * 2;

  // Just a few sliders for important FX
  AddLayoutSlider(L"Bloom Int:", IDC_SLIDER_FX_BLOOM_INT,
                  IDC_STATIC_FX_BLOOM_INT);
  AddLayoutSlider(L"Glow Size:", IDC_SLIDER_FX_GLOW_SIZE,
                  IDC_STATIC_FX_GLOW_SIZE);
  AddLayoutSlider(L"Distort Amt:", IDC_SLIDER_FX_DISTORT_AMT,
                  IDC_STATIC_FX_DISTORT_AMT);
  AddLayoutSlider(L"Trail Fade:", IDC_SLIDER_FX_TRAIL_FADE,
                  IDC_STATIC_FX_TRAIL_FADE);

  // Buttons at bottom
  AddItem(BS_DEFPUSHBUTTON | WS_TABSTOP, 200, 275, 80, 20, IDOK, L"BUTTON",
          L"OK");
  AddItem(BS_PUSHBUTTON | WS_TABSTOP, 290, 275, 80, 20, IDCANCEL, L"BUTTON",
          L"Cancel");
  AddItem(BS_PUSHBUTTON | WS_TABSTOP, 10, 275, 80, 20, IDC_BTN_DEFAULTS,
          L"BUTTON", L"Defaults");

  return (LPCDLGTEMPLATE)buffer;
}

bool ShowSettingsDialog(HINSTANCE hInstance, HWND parent, Config &config) {
  g_pConfig = &config;
  LPCDLGTEMPLATE lpdt = CreateDialogTemplate();
  INT_PTR result =
      DialogBoxIndirect(hInstance, lpdt, parent, SettingsDialogProc);
  return result == IDOK;
}
