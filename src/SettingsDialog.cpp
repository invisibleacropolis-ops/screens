#include "SettingsDialog.h"
#include "resource.h"
#include <commctrl.h>
#include <cstdio>

#pragma comment(lib, "comctl32.lib")

// Global config pointer for dialog proc
static Config *g_pConfig = nullptr;
static Config g_tempConfig;

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

// Initialize dialog controls from config
static void InitDialogFromConfig(HWND hDlg, const Config &cfg) {
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
}

// Read config from dialog controls
static void ReadConfigFromDialog(HWND hDlg, Config &cfg) {
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

    // Update corresponding label based on slider
    switch (id) {
    case IDC_SLIDER_BLOOM_THRESH:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_BLOOM_THRESH, true, 0.3f, 1.5f);
      break;
    case IDC_SLIDER_BLOOM_STR:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_BLOOM_STR, true, 0.0f, 2.0f);
      break;
    case IDC_SLIDER_EXPOSURE:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_EXPOSURE, true, 0.5f, 3.0f);
      break;
    case IDC_SLIDER_FOG_DENSITY:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_FOG_DENSITY, true, 0.0f, 0.2f);
      break;
    case IDC_SLIDER_PARTICLE_CNT:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_PARTICLE_CNT, false);
      break;
    case IDC_SLIDER_ROTATION:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_ROTATION, true, 0.0f, 2.0f);
      break;
    case IDC_SLIDER_CAM_DIST:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_CAM_DIST, true, 5.0f, 25.0f);
      break;
    case IDC_SLIDER_CAM_HEIGHT:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_CAM_HEIGHT, true, 0.0f, 15.0f);
      break;
    case IDC_SLIDER_FOV:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_FOV, false);
      break;
    case IDC_SLIDER_BG_R:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_BG_R, false);
      break;
    case IDC_SLIDER_BG_G:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_BG_G, false);
      break;
    case IDC_SLIDER_BG_B:
      UpdateSliderLabel(hDlg, id, IDC_STATIC_BG_B, false);
      break;
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
      g_tempConfig = Config(); // Reset to defaults
      InitDialogFromConfig(hDlg, g_tempConfig);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

// Create dialog template in memory (no .rc file needed)
static LPCDLGTEMPLATE CreateDialogTemplate() {
  // Dialog box template with all controls
  // We'll create a simple layout programmatically
  static BYTE buffer[8192] = {0};
  LPWORD p = (LPWORD)buffer;

  // Dialog header
  LPDLGTEMPLATE pDlg = (LPDLGTEMPLATE)p;
  pDlg->style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU |
                WS_VISIBLE;
  pDlg->dwExtendedStyle = 0;
  pDlg->cdit = 0; // Will count items
  pDlg->x = 0;
  pDlg->y = 0;
  pDlg->cx = 320;
  pDlg->cy = 380;

  p = (LPWORD)(pDlg + 1);
  *p++ = 0; // No menu
  *p++ = 0; // Default class

  // Title
  const wchar_t *title = L"Screensaver Settings";
  wcscpy_s((wchar_t *)p, 64, title);
  p += wcslen(title) + 1;

  // Align to DWORD
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

    // Class
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
      // Custom class name (like msctls_trackbar32)
      wcscpy_s((wchar_t *)p, 64, cls);
      p += wcslen(cls) + 1;
    }

    // Text
    if (text) {
      wcscpy_s((wchar_t *)p, 128, text);
      p += wcslen(text) + 1;
    } else {
      *p++ = 0;
    }

    *p++ = 0; // No creation data
    pDlg->cdit++;
  };

  int y = 10;
  int labelW = 100;
  int sliderW = 130;
  int valueW = 40;
  int rowH = 18;

  // Quality section
  AddItem(SS_LEFT, 10, y, labelW, 12, IDC_LABEL_QUALITY, L"STATIC",
          L"Quality:");
  AddItem(CBS_DROPDOWNLIST | WS_TABSTOP, 115, y - 2, 80, 100, IDC_COMBO_QUALITY,
          L"COMBOBOX", nullptr);
  y += rowH + 5;

  // -- Visual Effects Header --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 295, 1, 0, L"STATIC", nullptr);
  y += 5;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Visual Effects");
  y += rowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 80, 12, IDC_CHECK_BLOOM,
          L"BUTTON", L"Bloom");
  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 100, y, 80, 12, IDC_CHECK_FXAA,
          L"BUTTON", L"FXAA");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_BLOOM_THRESH, L"STATIC",
          L"Bloom Threshold:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_BLOOM_THRESH, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_BLOOM_THRESH, L"STATIC",
          L"0.70");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_BLOOM_STR, L"STATIC",
          L"Bloom Strength:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_BLOOM_STR, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_BLOOM_STR, L"STATIC",
          L"0.80");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_EXPOSURE, L"STATIC",
          L"Exposure:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_EXPOSURE, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_EXPOSURE, L"STATIC", L"1.00");
  y += rowH + 5;

  // -- Fog Header --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 295, 1, 0, L"STATIC", nullptr);
  y += 5;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Fog");
  y += rowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 80, 12, IDC_CHECK_FOG, L"BUTTON",
          L"Enable Fog");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_FOG_DENSITY, L"STATIC",
          L"Fog Density:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_FOG_DENSITY, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_FOG_DENSITY, L"STATIC",
          L"0.05");
  y += rowH + 5;

  // -- Particles Header --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 295, 1, 0, L"STATIC", nullptr);
  y += 5;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Particles");
  y += rowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 80, 12, IDC_CHECK_PARTICLES,
          L"BUTTON", L"Enable");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_PARTICLE_CNT, L"STATIC",
          L"Count:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_PARTICLE_CNT, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_PARTICLE_CNT, L"STATIC",
          L"6000");
  y += rowH + 5;

  // -- Camera Header --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 295, 1, 0, L"STATIC", nullptr);
  y += 5;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Camera & Scene");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_ROTATION, L"STATIC",
          L"Rotation Speed:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_ROTATION, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_ROTATION, L"STATIC", L"0.20");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_CAM_DIST, L"STATIC",
          L"Distance:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_CAM_DIST, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_CAM_DIST, L"STATIC",
          L"12.00");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_CAM_HEIGHT, L"STATIC",
          L"Height:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_CAM_HEIGHT, TRACKBAR_CLASSW,
          nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_CAM_HEIGHT, L"STATIC",
          L"5.00");
  y += rowH;

  AddItem(SS_LEFT, 15, y, labelW - 10, 12, IDC_LABEL_FOV, L"STATIC",
          L"Field of View:");
  AddItem(0, 115, y, sliderW, 15, IDC_SLIDER_FOV, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 250, y, valueW, 12, IDC_STATIC_FOV, L"STATIC", L"45");
  y += rowH + 5;

  // -- Background Header --
  AddItem(SS_LEFT | SS_SUNKEN, 10, y, 295, 1, 0, L"STATIC", nullptr);
  y += 5;
  AddItem(SS_LEFT, 10, y, 100, 12, 0, L"STATIC", L"Background");
  y += rowH;

  AddItem(BS_AUTOCHECKBOX | WS_TABSTOP, 15, y, 100, 12, IDC_CHECK_SKYBOX,
          L"BUTTON", L"Show Skybox");
  y += rowH;

  AddItem(SS_LEFT, 15, y, 25, 12, IDC_LABEL_BG_R, L"STATIC", L"R:");
  AddItem(0, 40, y, 60, 15, IDC_SLIDER_BG_R, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 105, y, 25, 12, IDC_STATIC_BG_R, L"STATIC", L"0");

  AddItem(SS_LEFT, 130, y, 25, 12, IDC_LABEL_BG_G, L"STATIC", L"G:");
  AddItem(0, 155, y, 60, 15, IDC_SLIDER_BG_G, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 220, y, 25, 12, IDC_STATIC_BG_G, L"STATIC", L"13");

  AddItem(SS_LEFT, 245, y, 25, 12, IDC_LABEL_BG_B, L"STATIC", L"B:");
  AddItem(0, 260, y, 35, 15, IDC_SLIDER_BG_B, TRACKBAR_CLASSW, nullptr);
  AddItem(SS_LEFT, 297, y, 25, 12, IDC_STATIC_BG_B, L"STATIC", L"25");
  y += rowH + 10;

  // Buttons
  AddItem(BS_DEFPUSHBUTTON | WS_TABSTOP, 80, y, 60, 18, IDOK, L"BUTTON", L"OK");
  AddItem(BS_PUSHBUTTON | WS_TABSTOP, 145, y, 60, 18, IDCANCEL, L"BUTTON",
          L"Cancel");
  AddItem(BS_PUSHBUTTON | WS_TABSTOP, 210, y, 60, 18, IDC_BTN_DEFAULTS,
          L"BUTTON", L"Defaults");

  // Update dialog height
  pDlg->cy = y + 30;

  return (LPCDLGTEMPLATE)buffer;
}

bool ShowSettingsDialog(HINSTANCE hInstance, HWND parent, Config &config) {
  // Init common controls for trackbars
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
