#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include "Config.h"
#include "SettingsDialog.h"
#include "renderer.h"
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <windows.h>

// Globals
HINSTANCE hInst;
const wchar_t *szTitle = L"Simple OpenGL Screensaver";
const wchar_t *szWindowClass = L"ScreenSaverClass";

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ParseCommandLine(LPWSTR cmdLine, char &mode, HWND &parentHwnd);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine, int nCmdShow) {
  hInst = hInstance;

  char mode = 's'; // Default to screensaver mode
  HWND parentHwnd = NULL;

  ParseCommandLine(lpCmdLine, mode, parentHwnd);

  if (mode == 'c') {
    Config config = LoadConfig();
    if (ShowSettingsDialog(hInstance, NULL, config)) {
      if (!SaveConfig(config)) {
        MessageBoxW(NULL, L"Failed to save configuration.", szTitle,
                    MB_OK | MB_ICONERROR);
      }
    }
    return 0;
  }

  // Register class
  WNDCLASSEXW wcex;
  wcex.cbSize = sizeof(WNDCLASSEXW);
  wcex.style =
      CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // OWNDC is important for OpenGL
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = NULL;
  wcex.hCursor = LoadCursor(
      NULL,
      IDC_ARROW); // Usually NULL for screensavers, but arrow is fine for now
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = NULL;

  if (!RegisterClassExW(&wcex)) {
    return 1;
  }

  // Create window
  HWND hWnd;
  if (mode == 'p' && parentHwnd) {
    // Preview mode: Child window
    RECT ptrRect;
    GetClientRect(parentHwnd, &ptrRect);

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_CHILD | WS_VISIBLE, 0, 0,
                         ptrRect.right, ptrRect.bottom, parentHwnd, NULL,
                         hInstance, NULL);
  } else {
    // Fullscreen screensaver mode
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // Hide cursor
    ShowCursor(FALSE);

    hWnd = CreateWindowW(szWindowClass, szTitle,
                         WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, width,
                         height, NULL, NULL, hInstance, NULL);
  }

  if (!hWnd)
    return 2;

  Config config = LoadConfig();
  SetConfig(config);
  InitOpenGL(hWnd);

  // Main loop
  MSG msg;
  bool running = true;
  while (running) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        running = false;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      // Render
      RECT rect;
      GetClientRect(hWnd, &rect);
      DrawScene(rect.right, rect.bottom);
      // Limit framerate slightly to avoid burning GPU in preview?
      // For now, let it run (SwapBuffers implicitly vsyncs usually)
    }
  }

  CleanupOpenGL(hWnd);

  return (int)msg.wParam;
}

// Global mouse tracker
POINT lastMousePos;
bool firstMouse = true;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
  case WM_CREATE:
    GetCursorPos(&lastMousePos);
    // Start a timer if we want updates without rendering loop, but we use a
    // loop in WinMain. Screen savers often set a timer to "system" checks.
    SetTimer(hWnd, 1, 10, NULL);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_TIMER:
    // Optional: Could verify if screensaver should close
    break;

  case WM_KEYDOWN:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    // Close on input
    // Note: For preview mode, we might not want to close on simple inputs
    // passed to child? Actually windows doesn't usually send input to the
    // preview child. We need to check if we are in preview mode or not.
    // Simplest way: if WS_CHILD is set, don't exit.
    if (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) {
      break;
    }
    PostQuitMessage(0);
    break;

  case WM_MOUSEMOVE:
    if (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) {
      break;
    }

    POINT pt;
    GetCursorPos(&pt);
    // Allow a small threshold or initial jitter
    if (firstMouse) {
      lastMousePos = pt;
      firstMouse = false;
    } else {
      if (abs(pt.x - lastMousePos.x) > 5 || abs(pt.y - lastMousePos.y) > 5) {
        PostQuitMessage(0);
      }
    }
    break;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

void ParseCommandLine(LPWSTR cmdLine, char &mode, HWND &parentHwnd) {
  // Basic parser.
  // Arguments are usually: /s, -s, /c, /p <hwnd>
  // Note: WinMain lpCmdLine doesn't include program name.

  std::wstring args = cmdLine;
  if (args.empty()) {
    mode = 'c'; // Default to config if no args (standard behavior differs,
                // sometimes /s)
    return;
  }

  // Checking for substring flags
  if (args.find(L"/s") != std::wstring::npos ||
      args.find(L"-s") != std::wstring::npos) {
    mode = 's';
  } else if (args.find(L"/c") != std::wstring::npos ||
             args.find(L"-c") != std::wstring::npos) {
    mode = 'c';
  } else if (args.find(L"/p") != std::wstring::npos ||
             args.find(L"-p") != std::wstring::npos) {
    mode = 'p';
    // Need to find the HWND. It usually follows /p
    // Look for next space
    size_t pPos = args.find(L"/p");
    if (pPos == std::wstring::npos)
      pPos = args.find(L"-p");

    // Skip "/p"
    if (pPos != std::wstring::npos) {
      std::wstring afterP = args.substr(pPos + 2);
      // trim leading spaces
      size_t firstNum = afterP.find_first_not_of(L" ");
      if (firstNum != std::wstring::npos) {
        std::wstring hwndStr = afterP.substr(firstNum);
        // Convert string to (unsigned) long, then cast to HWND
        // Screensaver pass hwnd as decimal unsigned integer usually.
        parentHwnd = (HWND)wcstoul(hwndStr.c_str(), NULL, 10);
      }
    }
  }
}
