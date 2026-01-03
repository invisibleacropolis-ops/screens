#pragma once

#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

void InitOpenGL(HWND hwnd);
void DrawScene(int width, int height);
void CleanupOpenGL(HWND hwnd);
