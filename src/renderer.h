#pragma once

#include "SystemMonitor.h"
#include <windows.h>

#include "glad/glad.h"


void InitOpenGL(HWND hwnd);
void DrawScene(int width, int height);
void CleanupOpenGL(HWND hwnd);

// Helper drawing functions
void DrawCube(float size, float r, float g, float b);
