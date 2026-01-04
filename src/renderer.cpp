#include "renderer.h"
#include <math.h>
#include <vector>

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif
#ifndef WGL_CONTEXT_MINOR_VERSION_ARB
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif
#ifndef WGL_CONTEXT_FLAGS_ARB
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#endif
#ifndef WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#endif
#ifndef WGL_CONTEXT_CORE_PROFILE_BIT_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif
#ifndef WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif
#ifndef WGL_DRAW_TO_WINDOW_ARB
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#endif
#ifndef WGL_SUPPORT_OPENGL_ARB
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#endif
#ifndef WGL_DOUBLE_BUFFER_ARB
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#endif
#ifndef WGL_PIXEL_TYPE_ARB
#define WGL_PIXEL_TYPE_ARB 0x2013
#endif
#ifndef WGL_COLOR_BITS_ARB
#define WGL_COLOR_BITS_ARB 0x2014
#endif
#ifndef WGL_DEPTH_BITS_ARB
#define WGL_DEPTH_BITS_ARB 0x2022
#endif
#ifndef WGL_STENCIL_BITS_ARB
#define WGL_STENCIL_BITS_ARB 0x2023
#endif
#ifndef WGL_TYPE_RGBA_ARB
#define WGL_TYPE_RGBA_ARB 0x202B
#endif
#ifndef WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9
#endif
#ifndef WGL_SAMPLE_BUFFERS_ARB
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#endif
#ifndef WGL_SAMPLES_ARB
#define WGL_SAMPLES_ARB 0x2042
#endif

typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC,
                                                        const int *);
typedef BOOL(WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int *,
                                                     const FLOAT *, UINT, int *,
                                                     UINT *);

static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;

HGLRC hRC = NULL;
HDC hDC = NULL;
SystemMonitor *sysMon = nullptr;

GLUquadricObj *quadric = nullptr;

static bool LoadWGLExtensions() {
  if (wglCreateContextAttribsARB && wglChoosePixelFormatARB) {
    return true;
  }

  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = _T("WGLDummyWindow");

  RegisterClass(&wc);

  HWND dummyWindow = CreateWindow(wc.lpszClassName, _T(""),
                                  WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL,
                                  wc.hInstance, NULL);
  if (!dummyWindow) {
    return false;
  }

  HDC dummyDC = GetDC(dummyWindow);
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(dummyDC, &pfd);
  SetPixelFormat(dummyDC, pixelFormat, &pfd);

  HGLRC dummyRC = wglCreateContext(dummyDC);
  wglMakeCurrent(dummyDC, dummyRC);

  wglCreateContextAttribsARB =
      (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress(
          "wglCreateContextAttribsARB");
  wglChoosePixelFormatARB =
      (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress(
          "wglChoosePixelFormatARB");

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(dummyRC);
  ReleaseDC(dummyWindow, dummyDC);
  DestroyWindow(dummyWindow);
  UnregisterClass(wc.lpszClassName, wc.hInstance);

  return wglCreateContextAttribsARB && wglChoosePixelFormatARB;
}

static bool SetPixelFormatWithAttributes(HDC dc, bool requestSrgb,
                                         bool requestMsaa,
                                         bool &srgbEnabled) {
  if (!wglChoosePixelFormatARB) {
    return false;
  }

  std::vector<int> attribs = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB, 24,
      WGL_DEPTH_BITS_ARB, 24,
      WGL_STENCIL_BITS_ARB, 8};

  if (requestSrgb) {
    attribs.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
    attribs.push_back(GL_TRUE);
  }

  if (requestMsaa) {
    attribs.push_back(WGL_SAMPLE_BUFFERS_ARB);
    attribs.push_back(1);
    attribs.push_back(WGL_SAMPLES_ARB);
    attribs.push_back(4);
  }

  attribs.push_back(0);
  attribs.push_back(0);

  int format = 0;
  UINT numFormats = 0;
  if (!wglChoosePixelFormatARB(dc, attribs.data(), NULL, 1, &format,
                               &numFormats) ||
      numFormats == 0) {
    return false;
  }

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof(pfd));
  DescribePixelFormat(dc, format, sizeof(pfd), &pfd);
  if (!SetPixelFormat(dc, format, &pfd)) {
    return false;
  }

  srgbEnabled = requestSrgb;
  return true;
}

static bool SetPixelFormatWithFallbacks(HDC dc, bool &srgbEnabled) {
  srgbEnabled = false;

  if (SetPixelFormatWithAttributes(dc, true, true, srgbEnabled)) {
    return true;
  }
  if (SetPixelFormatWithAttributes(dc, true, false, srgbEnabled)) {
    return true;
  }
  if (SetPixelFormatWithAttributes(dc, false, true, srgbEnabled)) {
    return true;
  }
  if (SetPixelFormatWithAttributes(dc, false, false, srgbEnabled)) {
    return true;
  }

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(dc, &pfd);
  if (pixelFormat == 0) {
    return false;
  }

  return SetPixelFormat(dc, pixelFormat, &pfd);
}

static HGLRC CreateCoreContext(HDC dc, int major, int minor) {
  if (!wglCreateContextAttribsARB) {
    return NULL;
  }

  int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                   major,
                   WGL_CONTEXT_MINOR_VERSION_ARB,
                   minor,
                   WGL_CONTEXT_PROFILE_MASK_ARB,
                   WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                   WGL_CONTEXT_FLAGS_ARB,
                   0,
                   0};

  return wglCreateContextAttribsARB(dc, 0, attribs);
}

static HGLRC CreateCompatibilityContext(HDC dc, int major, int minor) {
  if (!wglCreateContextAttribsARB) {
    return NULL;
  }

  int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                   major,
                   WGL_CONTEXT_MINOR_VERSION_ARB,
                   minor,
                   WGL_CONTEXT_PROFILE_MASK_ARB,
                   WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                   WGL_CONTEXT_FLAGS_ARB,
                   0,
                   0};

  return wglCreateContextAttribsARB(dc, 0, attribs);
}

void InitOpenGL(HWND hwnd) {
  hDC = GetDC(hwnd);

  LoadWGLExtensions();

  bool srgbEnabled = false;
  if (!SetPixelFormatWithFallbacks(hDC, srgbEnabled)) {
    return;
  }

  hRC = CreateCoreContext(hDC, 4, 6);
  if (!hRC) {
    hRC = CreateCoreContext(hDC, 4, 5);
  }
  if (!hRC) {
    hRC = CreateCompatibilityContext(hDC, 4, 6);
  }
  if (!hRC) {
    hRC = CreateCompatibilityContext(hDC, 4, 5);
  }
  if (!hRC) {
    hRC = wglCreateContext(hDC);
  }

  if (!hRC) {
    return;
  }

  wglMakeCurrent(hDC, hRC);

  if (!gladLoadGL()) {
    OutputDebugStringA("GLAD failed to initialize.\n");
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  if (srgbEnabled) {
    glEnable(GL_FRAMEBUFFER_SRGB);
  }

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
