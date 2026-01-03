#include "renderer.h"

HGLRC hRC = NULL;
HDC hDC = NULL;

void InitOpenGL(HWND hwnd) {
  hDC = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int iFormat = ChoosePixelFormat(hDC, &pfd);
  SetPixelFormat(hDC, iFormat, &pfd);

  hRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hRC);

  // Basic setup
  glEnable(GL_DEPTH_TEST);
}

void DrawScene(int width, int height) {
  if (height == 0)
    height = 1;
  glViewport(0, 0, width, height);

  // Clear
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Dark blue background
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

  // Model view
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -6.0f);

  // Rotate
  static float theta = 0.0f;
  theta += 1.0f;
  glRotatef(theta, 0.0f, 1.0f, 0.0f);

  // Draw triangle
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-1.0f, -1.0f, 0.0f);
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 0.0f);
  glEnd();

  SwapBuffers(hDC);
}

void CleanupOpenGL(HWND hwnd) {
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);
  ReleaseDC(hwnd, hDC);
}
