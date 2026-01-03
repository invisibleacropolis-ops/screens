#include "renderer.h"
#include <math.h>

HGLRC hRC = NULL;
HDC hDC = NULL;
SystemMonitor *sysMon = nullptr;

GLUquadricObj *quadric = nullptr;

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

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  // Light Setup
  GLfloat light_pos[] = {10.0f, 10.0f, 10.0f, 0.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);

  sysMon = new SystemMonitor();
  sysMon->Initialize();

  quadric = gluNewQuadric();
}

void DrawCube(float size) {
  float h = size / 2.0f;
  glBegin(GL_QUADS);
  // Front
  glNormal3f(0.0f, 0.0f, 1.0f);
  glVertex3f(-h, -h, h);
  glVertex3f(h, -h, h);
  glVertex3f(h, h, h);
  glVertex3f(-h, h, h);
  // Back
  glNormal3f(0.0f, 0.0f, -1.0f);
  glVertex3f(-h, -h, -h);
  glVertex3f(-h, h, -h);
  glVertex3f(h, h, -h);
  glVertex3f(h, -h, -h);
  // Top
  glNormal3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-h, h, -h);
  glVertex3f(-h, h, h);
  glVertex3f(h, h, h);
  glVertex3f(h, h, -h);
  // Bottom
  glNormal3f(0.0f, -1.0f, 0.0f);
  glVertex3f(-h, -h, -h);
  glVertex3f(h, -h, -h);
  glVertex3f(h, -h, h);
  glVertex3f(-h, -h, h);
  // Right
  glNormal3f(1.0f, 0.0f, 0.0f);
  glVertex3f(h, -h, -h);
  glVertex3f(h, h, -h);
  glVertex3f(h, h, h);
  glVertex3f(h, -h, h);
  // Left
  glNormal3f(-1.0f, 0.0f, 0.0f);
  glVertex3f(-h, -h, -h);
  glVertex3f(-h, -h, h);
  glVertex3f(-h, h, h);
  glVertex3f(-h, h, -h);
  glEnd();
}

// CPU Visualization: A pulsing sphere
void DrawCPU(float usage) {
  glPushMatrix();
  // Normalize usage 0.0 - 1.0
  float u = usage / 100.0f;

  // Color: Blue (Low) -> Red (High)
  glColor3f(u, 0.2f, 1.0f - u);

  // Pulse scale: Small (Low) -> Large (High) + heartbeat
  static float pulse = 0.0f;
  pulse += 0.1f + (u * 0.5f); // Beat faster with load
  float scale = 1.0f + (u * 0.5f) + (sin(pulse) * 0.1f);

  glScalef(scale, scale, scale);

  if (quadric) {
    gluSphere(quadric, 1.0f, 32, 32);
  }
  glPopMatrix();
}

// RAM Visualization: A grid of cubes that fills up
void DrawRAM(float usage) {
  glPushMatrix();
  glTranslatef(0.0f, -2.5f, 0.0f); // Move below CPU

  // 10x10 Grid
  float u = usage / 100.0f; // 0..1
  int totalCubes = 100;
  int litCubes = (int)(u * 100);

  float spacing = 0.25f;
  float startX = -(10 * spacing) / 2.0f;
  float startZ = -(10 * spacing) / 2.0f;

  for (int z = 0; z < 10; z++) {
    for (int x = 0; x < 10; x++) {
      int idx = z * 10 + x;
      glPushMatrix();
      glTranslatef(startX + x * spacing, 0.0f, startZ + z * spacing);

      if (idx < litCubes) {
        // Active RAM: Green-ish / Orange-ish
        glColor3f(u > 0.8f ? 1.0f : 0.0f, 1.0f - u, 0.0f);
        DrawCube(0.2f);
      } else {
        // Empty RAM: Dark grey outline/wireframe or dim
        glColor3f(0.1f, 0.1f, 0.1f);
        DrawCube(0.1f);
      }
      glPopMatrix();
    }
  }
  glPopMatrix();
}

// Disk Visualization: A rotating ring around the CPU
void DrawDisk(float usage) {
  glPushMatrix();
  glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Flat ring

  // Spin based on usage? Actually disk usage is activity.
  // Let's just create a static ring that glows.
  static float rot = 0.0f;
  rot += 1.0f + (usage * 0.5f); // Spin faster
  glRotatef(rot, 0.0f, 0.0f, 1.0f);

  float u = usage / 100.0f;

  // Glow: White/Yellow
  glColor3f(0.5f + u * 0.5f, 0.5f + u * 0.5f, 0.0f);

  if (quadric) {
    // Ring
    gluDisk(quadric, 2.0f, 2.2f + (u * 0.5f), 32, 1);
  }
  glPopMatrix();
}

void DrawScene(int width, int height) {
  if (sysMon)
    sysMon->Update();

  if (height == 0)
    height = 1;
  glViewport(0, 0, width, height);

  // Clear
  glClearColor(0.0f, 0.05f, 0.1f, 1.0f); // Dark sci-fi blue
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

  // Camera
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0f, 5.0f, 12.0f, // Eye
            0.0f, 0.0f, 0.0f,  // Target
            0.0f, 1.0f, 0.0f); // Up

  // Global Rotate Scene slightly
  static float sceneRot = 0.0f;
  sceneRot += 0.2f;
  glRotatef(sceneRot, 0.0f, 1.0f, 0.0f);

  if (sysMon) {
    DrawCPU((float)sysMon->GetCpuUsage());
    DrawRAM((float)sysMon->GetRamUsage());
    DrawDisk((float)sysMon->GetDiskUsage());
  }

  SwapBuffers(hDC);
}

void CleanupOpenGL(HWND hwnd) {
  if (quadric)
    gluDeleteQuadric(quadric);
  if (sysMon)
    delete sysMon;

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);
  ReleaseDC(hwnd, hDC);
}
