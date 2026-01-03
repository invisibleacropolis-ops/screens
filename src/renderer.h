#pragma once

#include "SystemMonitor.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <windows.h>


void InitOpenGL(HWND hwnd);
void DrawScene(int width, int height);
void CleanupOpenGL(HWND hwnd);

// Helper drawing functions
void DrawCube(float size);
void DrawSphere(float radius, int slices, int stacks);
void DrawGrid(float size, int steps);
