#include "renderer.h"
#include "Config.h"
#include "DiagTest.h"
#include "Particles.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include <array>
#include <cmath>
#include <cstring>
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
Particles *gParticles = nullptr;
Config gConfig{};

struct Vec3 {
  float x;
  float y;
  float z;
};

struct Mat4 {
  std::array<float, 16> m;
};

struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  GLsizei indexCount = 0;
  GLsizei vertexCount = 0;
  bool indexed = false;
};

struct PostProcessPipeline {
  int width = 0;
  int height = 0;
  GLuint hdrFbo = 0;
  GLuint hdrColor = 0;
  GLuint hdrDepth = 0;
  GLuint ldrFbo = 0;
  GLuint ldrColor = 0;
  GLuint pingpongFbo[2] = {0, 0};
  GLuint pingpongColor[2] = {0, 0};
  GLuint quadVao = 0;
  GLuint quadVbo = 0;
  Shader *extractShader = nullptr;
  Shader *blurShader = nullptr;
  Shader *tonemapShader = nullptr;
  Shader *fxaaShader = nullptr;
  bool fxaaEnabled = true;
  bool bloomEnabled = true;
};

Shader *gShader = nullptr;
Shader *gSkyboxShader = nullptr;
GLuint gSceneUbo = 0;
GLuint gEnvironmentMap = 0;
Mesh gCubeMesh;
Mesh gSphereMesh;
Mesh gRingMesh;
static constexpr float kPi = 3.14159265358979323846f;
Mat4 gSceneTransform{};
PostProcessPipeline gPost{};
static LARGE_INTEGER gFrameTimerFreq{};
static LARGE_INTEGER gLastFrameTime{};
static bool gHasFrameTime = false;

struct SceneUniforms {
  float view[16];
  float proj[16];
  float lightDirAndAmbient[4];
  float cameraPos[4];
  float fogColor[4];
  float fogParams[4];
  float fogParams2[4];
};

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

  HWND dummyWindow = CreateWindow(wc.lpszClassName, _T(""), WS_OVERLAPPEDWINDOW,
                                  0, 0, 1, 1, NULL, NULL, wc.hInstance, NULL);
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
  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress(
      "wglChoosePixelFormatARB");

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(dummyRC);
  ReleaseDC(dummyWindow, dummyDC);
  DestroyWindow(dummyWindow);
  UnregisterClass(wc.lpszClassName, wc.hInstance);

  return wglCreateContextAttribsARB && wglChoosePixelFormatARB;
}

static bool SetPixelFormatWithAttributes(HDC dc, bool requestSrgb,
                                         bool requestMsaa, bool &srgbEnabled) {
  if (!wglChoosePixelFormatARB) {
    return false;
  }

  std::vector<int> attribs = {WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                              WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                              WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
                              WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
                              WGL_COLOR_BITS_ARB,     24,
                              WGL_DEPTH_BITS_ARB,     24,
                              WGL_STENCIL_BITS_ARB,   8};

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

static Vec3 Vec3Sub(const Vec3 &a, const Vec3 &b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

static Vec3 Vec3Cross(const Vec3 &a, const Vec3 &b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static Vec3 Vec3Normalize(const Vec3 &v) {
  float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len == 0.0f) {
    return {0.0f, 0.0f, 0.0f};
  }
  return {v.x / len, v.y / len, v.z / len};
}

static Mat4 Mat4Identity() {
  Mat4 out{};
  out.m = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
           0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  return out;
}

static Mat4 Mat4Multiply(const Mat4 &a, const Mat4 &b) {
  Mat4 out{};
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      out.m[row + col * 4] =
          a.m[row + 0] * b.m[col * 4 + 0] + a.m[row + 4] * b.m[col * 4 + 1] +
          a.m[row + 8] * b.m[col * 4 + 2] + a.m[row + 12] * b.m[col * 4 + 3];
    }
  }
  return out;
}

static Mat4 Mat4Translate(float x, float y, float z) {
  Mat4 out = Mat4Identity();
  out.m[12] = x;
  out.m[13] = y;
  out.m[14] = z;
  return out;
}

static Mat4 Mat4Scale(float x, float y, float z) {
  Mat4 out = Mat4Identity();
  out.m[0] = x;
  out.m[5] = y;
  out.m[10] = z;
  return out;
}

static Mat4 Mat4RotateX(float radians) {
  Mat4 out = Mat4Identity();
  float c = std::cos(radians);
  float s = std::sin(radians);
  out.m[5] = c;
  out.m[6] = s;
  out.m[9] = -s;
  out.m[10] = c;
  return out;
}

static Mat4 Mat4RotateY(float radians) {
  Mat4 out = Mat4Identity();
  float c = std::cos(radians);
  float s = std::sin(radians);
  out.m[0] = c;
  out.m[2] = -s;
  out.m[8] = s;
  out.m[10] = c;
  return out;
}

static Mat4 Mat4RotateZ(float radians) {
  Mat4 out = Mat4Identity();
  float c = std::cos(radians);
  float s = std::sin(radians);
  out.m[0] = c;
  out.m[1] = s;
  out.m[4] = -s;
  out.m[5] = c;
  return out;
}

static Mat4 Mat4Perspective(float fovRadians, float aspect, float nearPlane,
                            float farPlane) {
  Mat4 out{};
  float f = 1.0f / std::tan(fovRadians / 2.0f);
  out.m = {f / aspect,
           0.0f,
           0.0f,
           0.0f,
           0.0f,
           f,
           0.0f,
           0.0f,
           0.0f,
           0.0f,
           (farPlane + nearPlane) / (nearPlane - farPlane),
           -1.0f,
           0.0f,
           0.0f,
           (2.0f * farPlane * nearPlane) / (nearPlane - farPlane),
           0.0f};
  return out;
}

static Mat4 Mat4LookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up) {
  Vec3 forward = Vec3Normalize(Vec3Sub(target, eye));
  Vec3 side = Vec3Normalize(Vec3Cross(forward, up));
  Vec3 upVec = Vec3Cross(side, forward);

  Mat4 out = Mat4Identity();
  out.m[0] = side.x;
  out.m[1] = side.y;
  out.m[2] = side.z;
  out.m[4] = upVec.x;
  out.m[5] = upVec.y;
  out.m[6] = upVec.z;
  out.m[8] = -forward.x;
  out.m[9] = -forward.y;
  out.m[10] = -forward.z;
  out.m[12] = -(side.x * eye.x + side.y * eye.y + side.z * eye.z);
  out.m[13] = -(upVec.x * eye.x + upVec.y * eye.y + upVec.z * eye.z);
  out.m[14] = (forward.x * eye.x + forward.y * eye.y + forward.z * eye.z);
  return out;
}

static Mesh CreateCubeMesh() {
  Mesh mesh{};
  float vertices[] = {
      // positions          // normals
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
      0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,
      0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
      0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f,
      0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
      0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,
      0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
      0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,
      0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,

      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
      1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,
      1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
      -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
      -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f};

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glBindVertexArray(0);

  mesh.vertexCount =
      static_cast<GLsizei>(sizeof(vertices) / (6 * sizeof(float)));
  mesh.indexed = false;
  return mesh;
}

static Mesh CreateSphereMesh(int slices, int stacks) {
  Mesh mesh{};
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  for (int stack = 0; stack <= stacks; ++stack) {
    float v = static_cast<float>(stack) / static_cast<float>(stacks);
    float phi = v * kPi;
    float cosPhi = std::cos(phi);
    float sinPhi = std::sin(phi);

    for (int slice = 0; slice <= slices; ++slice) {
      float u = static_cast<float>(slice) / static_cast<float>(slices);
      float theta = u * 2.0f * kPi;
      float cosTheta = std::cos(theta);
      float sinTheta = std::sin(theta);

      float x = sinPhi * cosTheta;
      float y = cosPhi;
      float z = sinPhi * sinTheta;

      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
    }
  }

  for (int stack = 0; stack < stacks; ++stack) {
    for (int slice = 0; slice < slices; ++slice) {
      int first = stack * (slices + 1) + slice;
      int second = first + slices + 1;

      indices.push_back(first);
      indices.push_back(second);
      indices.push_back(first + 1);
      indices.push_back(second);
      indices.push_back(second + 1);
      indices.push_back(first + 1);
    }
  }

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glBindVertexArray(0);

  mesh.indexCount = static_cast<GLsizei>(indices.size());
  mesh.indexed = true;
  return mesh;
}

static Mesh CreateRingMesh(int segments, float innerRadius, float outerRadius) {
  Mesh mesh{};
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  for (int i = 0; i <= segments; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(segments);
    float angle = t * 2.0f * kPi;
    float c = std::cos(angle);
    float s = std::sin(angle);

    float innerX = innerRadius * c;
    float innerY = innerRadius * s;
    float outerX = outerRadius * c;
    float outerY = outerRadius * s;

    vertices.insert(vertices.end(), {innerX, innerY, 0.0f, 0.0f, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {outerX, outerY, 0.0f, 0.0f, 0.0f, 1.0f});
  }

  for (int i = 0; i < segments; ++i) {
    unsigned int start = i * 2;
    indices.push_back(start);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 1);
    indices.push_back(start + 3);
    indices.push_back(start + 2);
  }

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glBindVertexArray(0);

  mesh.indexCount = static_cast<GLsizei>(indices.size());
  mesh.indexed = true;
  return mesh;
}

static void DestroyMesh(Mesh &mesh) {
  if (mesh.ebo) {
    glDeleteBuffers(1, &mesh.ebo);
  }
  if (mesh.vbo) {
    glDeleteBuffers(1, &mesh.vbo);
  }
  if (mesh.vao) {
    glDeleteVertexArrays(1, &mesh.vao);
  }
  mesh = {};
}

static void DrawMesh(const Mesh &mesh) {
  glBindVertexArray(mesh.vao);
  if (mesh.indexed) {
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
  }
  glBindVertexArray(0);
}

static GLuint CreateColorTexture(int width, int height, GLint internalFormat,
                                 GLenum format, GLenum type) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

static void DestroyPostProcessTargets(PostProcessPipeline &pipeline) {
  if (pipeline.hdrDepth) {
    glDeleteRenderbuffers(1, &pipeline.hdrDepth);
    pipeline.hdrDepth = 0;
  }
  if (pipeline.hdrColor) {
    glDeleteTextures(1, &pipeline.hdrColor);
    pipeline.hdrColor = 0;
  }
  if (pipeline.hdrFbo) {
    glDeleteFramebuffers(1, &pipeline.hdrFbo);
    pipeline.hdrFbo = 0;
  }
  if (pipeline.ldrColor) {
    glDeleteTextures(1, &pipeline.ldrColor);
    pipeline.ldrColor = 0;
  }
  if (pipeline.ldrFbo) {
    glDeleteFramebuffers(1, &pipeline.ldrFbo);
    pipeline.ldrFbo = 0;
  }
  for (int i = 0; i < 2; ++i) {
    if (pipeline.pingpongColor[i]) {
      glDeleteTextures(1, &pipeline.pingpongColor[i]);
      pipeline.pingpongColor[i] = 0;
    }
    if (pipeline.pingpongFbo[i]) {
      glDeleteFramebuffers(1, &pipeline.pingpongFbo[i]);
      pipeline.pingpongFbo[i] = 0;
    }
  }
  pipeline.width = 0;
  pipeline.height = 0;
}

static void CreatePostProcessTargets(PostProcessPipeline &pipeline, int width,
                                     int height) {
  if (pipeline.width == width && pipeline.height == height && pipeline.hdrFbo) {
    return;
  }

  DestroyPostProcessTargets(pipeline);
  pipeline.width = width;
  pipeline.height = height;

  glGenFramebuffers(1, &pipeline.hdrFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, pipeline.hdrFbo);
  pipeline.hdrColor =
      CreateColorTexture(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         pipeline.hdrColor, 0);
  glGenRenderbuffers(1, &pipeline.hdrDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, pipeline.hdrDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, pipeline.hdrDepth);

  glGenFramebuffers(1, &pipeline.ldrFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, pipeline.ldrFbo);
  pipeline.ldrColor =
      CreateColorTexture(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         pipeline.ldrColor, 0);

  glGenFramebuffers(2, pipeline.pingpongFbo);
  glGenTextures(2, pipeline.pingpongColor);
  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline.pingpongFbo[i]);
    glBindTexture(GL_TEXTURE_2D, pipeline.pingpongColor[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           pipeline.pingpongColor[i], 0);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void CreateFullScreenQuad(PostProcessPipeline &pipeline) {
  if (pipeline.quadVao) {
    return;
  }

  float quadVertices[] = {// positions   // uv
                          -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 0.0f,
                          1.0f,  1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                          1.0f,  1.0f,  1.0f, 1.0f, -1.0f, 1.0f,  0.0f, 1.0f};

  glGenVertexArrays(1, &pipeline.quadVao);
  glGenBuffers(1, &pipeline.quadVbo);
  glBindVertexArray(pipeline.quadVao);
  glBindBuffer(GL_ARRAY_BUFFER, pipeline.quadVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        reinterpret_cast<void *>(2 * sizeof(float)));
  glBindVertexArray(0);
}

static void DrawFullScreenQuad(const PostProcessPipeline &pipeline) {
  glBindVertexArray(pipeline.quadVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
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

  // Run diagnostics to identify issues
  DiagTest::RunAllDiagnostics();

  glEnable(GL_DEPTH_TEST);

  if (srgbEnabled) {
    glEnable(GL_FRAMEBUFFER_SRGB);
  }

  gShader =
      new Shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
  gSkyboxShader = new Shader("assets/shaders/environment.vert",
                             "assets/shaders/environment.frag");

  glGenBuffers(1, &gSceneUbo);
  glBindBuffer(GL_UNIFORM_BUFFER, gSceneUbo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUniforms), nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, gSceneUbo, 0, sizeof(SceneUniforms));

  if (gShader && gShader->IsValid()) {
    gShader->Use();
    gShader->BindUniformBlock("SceneData", 0);
    gShader->SetInt("uEnvironmentMap", 0);
  }

  if (gSkyboxShader && gSkyboxShader->IsValid()) {
    gSkyboxShader->Use();
    gSkyboxShader->BindUniformBlock("SceneData", 0);
    gSkyboxShader->SetInt("uEnvironment", 0);
  }

  const std::array<std::string, 6> skyboxFaces = {
      "assets/textures/px.ppm", "assets/textures/nx.ppm",
      "assets/textures/py.ppm", "assets/textures/ny.ppm",
      "assets/textures/pz.ppm", "assets/textures/nz.ppm"};
  gEnvironmentMap = Texture::LoadCubeMap(skyboxFaces);

  sysMon = new SystemMonitor();
  sysMon->Initialize();

  QueryPerformanceFrequency(&gFrameTimerFreq);
  gHasFrameTime = false;

  gCubeMesh = CreateCubeMesh();
  gSphereMesh = CreateSphereMesh(32, 16);
  gRingMesh = CreateRingMesh(64, 2.0f, 2.2f);

  const int particleCount = GetParticleCount(gConfig);
  if (particleCount > 0) {
    gParticles = new Particles(static_cast<std::size_t>(particleCount));
    if (!gParticles->Initialize()) {
      delete gParticles;
      gParticles = nullptr;
    }
  }

  CreateFullScreenQuad(gPost);
  gPost.extractShader = new Shader("assets/shaders/fullscreen.vert",
                                   "assets/shaders/bloom_extract.frag");
  gPost.blurShader = new Shader("assets/shaders/fullscreen.vert",
                                "assets/shaders/bloom_blur.frag");
  gPost.tonemapShader = new Shader("assets/shaders/fullscreen.vert",
                                   "assets/shaders/tonemap.frag");
  gPost.fxaaShader =
      new Shader("assets/shaders/fullscreen.vert", "assets/shaders/fxaa.frag");

  if (gPost.extractShader && gPost.extractShader->IsValid()) {
    gPost.extractShader->Use();
    gPost.extractShader->SetInt("uScene", 0);
  }
  if (gPost.blurShader && gPost.blurShader->IsValid()) {
    gPost.blurShader->Use();
    gPost.blurShader->SetInt("uImage", 0);
  }
  if (gPost.tonemapShader && gPost.tonemapShader->IsValid()) {
    gPost.tonemapShader->Use();
    gPost.tonemapShader->SetInt("uHdrTexture", 0);
    gPost.tonemapShader->SetInt("uBloomTexture", 1);
  }
  if (gPost.fxaaShader && gPost.fxaaShader->IsValid()) {
    gPost.fxaaShader->Use();
    gPost.fxaaShader->SetInt("uInput", 0);
  }
}

void DrawCube(float size, float r, float g, float b) {
  if (!gShader || !gShader->IsValid()) {
    return;
  }

  Mat4 model = Mat4Multiply(gSceneTransform, Mat4Scale(size, size, size));
  gShader->Use();
  gShader->SetMat4("uModel", model.m.data());
  gShader->SetVec3("uColor", r, g, b);
  DrawMesh(gCubeMesh);
}

// Helper to get the appropriate mesh based on MeshType
static Mesh &GetMeshByType(MeshType type) {
  switch (type) {
  case MeshType::Cube:
    return gCubeMesh;
  case MeshType::Ring:
    return gRingMesh;
  case MeshType::Sphere:
  default:
    return gSphereMesh;
  }
}

// CPU Visualization: A pulsing shape
void DrawCPU(float usage) {
  if (!gConfig.cpuMetric.enabled)
    return;
  if (gConfig.cpuMetric.meshType == MeshType::None)
    return;

  // Apply threshold - if usage is below threshold, reduce the effect
  float effective = usage - gConfig.cpuMetric.threshold;
  if (effective < 0.0f)
    effective = 0.0f;
  float u = (effective / (100.0f - gConfig.cpuMetric.threshold)) *
            gConfig.cpuMetric.strength;
  u = (u > 1.0f) ? 1.0f : u;

  static float pulse = 0.0f;
  pulse += 0.1f + (u * 0.5f);
  float scale = 1.0f + (u * 0.5f) + (std::sin(pulse) * 0.1f);

  if (!gShader || !gShader->IsValid()) {
    return;
  }

  Mat4 model = Mat4Multiply(gSceneTransform, Mat4Scale(scale, scale, scale));
  gShader->Use();
  gShader->SetMat4("uModel", model.m.data());
  gShader->SetVec3("uColor", u, 0.2f, 1.0f - u);
  DrawMesh(GetMeshByType(gConfig.cpuMetric.meshType));
}

// RAM Visualization: A grid of shapes that fills up
void DrawRAM(float usage) {
  if (!gConfig.ramMetric.enabled)
    return;
  if (gConfig.ramMetric.meshType == MeshType::None)
    return;
  if (!gShader || !gShader->IsValid()) {
    return;
  }

  // Apply threshold and strength
  float effective = usage - gConfig.ramMetric.threshold;
  if (effective < 0.0f)
    effective = 0.0f;
  float u = (effective / (100.0f - gConfig.ramMetric.threshold)) *
            gConfig.ramMetric.strength;
  u = (u > 1.0f) ? 1.0f : u;

  int totalCubes = 100;
  int litCubes = (int)(u * 100);

  float spacing = 0.25f;
  float startX = -(10 * spacing) / 2.0f;
  float startZ = -(10 * spacing) / 2.0f;

  for (int z = 0; z < 10; z++) {
    for (int x = 0; x < 10; x++) {
      int idx = z * 10 + x;
      float tx = startX + x * spacing;
      float tz = startZ + z * spacing;
      Mat4 translate = Mat4Translate(tx, -2.5f, tz);
      float size = (idx < litCubes) ? 0.2f : 0.1f;
      Mat4 scale = Mat4Scale(size, size, size);
      Mat4 model =
          Mat4Multiply(gSceneTransform, Mat4Multiply(translate, scale));

      gShader->Use();
      gShader->SetMat4("uModel", model.m.data());
      if (idx < litCubes) {
        gShader->SetVec3("uColor", u > 0.8f ? 1.0f : 0.0f, 1.0f - u, 0.0f);
      } else {
        gShader->SetVec3("uColor", 0.1f, 0.1f, 0.1f);
      }
      DrawMesh(GetMeshByType(gConfig.ramMetric.meshType));
    }
  }
}

// Disk Visualization: A rotating shape
void DrawDisk(float usage) {
  if (!gConfig.diskMetric.enabled)
    return;
  if (gConfig.diskMetric.meshType == MeshType::None)
    return;

  // Apply threshold and strength
  float effective = usage - gConfig.diskMetric.threshold;
  if (effective < 0.0f)
    effective = 0.0f;
  float u = (effective / (100.0f - gConfig.diskMetric.threshold)) *
            gConfig.diskMetric.strength;
  u = (u > 1.0f) ? 1.0f : u;

  static float rot = 0.0f;
  rot += 1.0f + (u * 50.0f);

  if (!gShader || !gShader->IsValid()) {
    return;
  }

  float ringScale = (2.2f + (u * 0.5f)) / 2.2f;
  Mat4 rotate =
      Mat4Multiply(Mat4RotateX(1.5707964f), Mat4RotateZ(rot * 0.0174533f));
  Mat4 scale = Mat4Scale(ringScale, ringScale, ringScale);
  Mat4 model = Mat4Multiply(gSceneTransform, Mat4Multiply(rotate, scale));

  gShader->Use();
  gShader->SetMat4("uModel", model.m.data());
  gShader->SetVec3("uColor", 0.5f + u * 0.5f, 0.5f + u * 0.5f, 0.0f);
  DrawMesh(GetMeshByType(gConfig.diskMetric.meshType));
}

void DrawScene(int width, int height) {
  LARGE_INTEGER now{};
  QueryPerformanceCounter(&now);
  float dtSeconds = 0.016f;
  if (gHasFrameTime && gFrameTimerFreq.QuadPart > 0) {
    dtSeconds =
        static_cast<float>((now.QuadPart - gLastFrameTime.QuadPart) /
                           static_cast<double>(gFrameTimerFreq.QuadPart));
  } else {
    gHasFrameTime = true;
  }
  gLastFrameTime = now;

  if (sysMon)
    sysMon->Update();

  if (height == 0)
    height = 1;
  glViewport(0, 0, width, height);

  bool fxaaReady = gPost.fxaaShader && gPost.fxaaShader->IsValid() &&
                   gPost.fxaaEnabled && gConfig.fxaaEnabled;
  bool bloomReady = gPost.extractShader && gPost.extractShader->IsValid() &&
                    gPost.blurShader && gPost.blurShader->IsValid() &&
                    gPost.bloomEnabled && gConfig.bloomEnabled;
  bool postReady = gPost.tonemapShader && gPost.tonemapShader->IsValid() &&
                   gPost.quadVao != 0;

  if (postReady) {
    CreatePostProcessTargets(gPost, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, gPost.hdrFbo);
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // Clear - use configured background color
  float bgR = gConfig.bgColorR / 255.0f;
  float bgG = gConfig.bgColorG / 255.0f;
  float bgB = gConfig.bgColorB / 255.0f;
  glClearColor(bgR, bgG, bgB, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!gShader || !gShader->IsValid()) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    SwapBuffers(hDC);
    return;
  }

  float aspect = static_cast<float>(width) / static_cast<float>(height);
  Mat4 projection = Mat4Perspective(gConfig.fieldOfView * (kPi / 180.0f),
                                    aspect, 0.1f, 100.0f);
  Vec3 cameraPos = {0.0f, gConfig.cameraHeight, gConfig.cameraDistance};
  Mat4 view = Mat4LookAt(cameraPos, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});

  static float sceneRot = 0.0f;
  sceneRot += gConfig.rotationSpeed;
  gSceneTransform = Mat4RotateY(sceneRot * (kPi / 180.0f));

  SceneUniforms scene{};
  std::memcpy(scene.view, view.m.data(), sizeof(scene.view));
  std::memcpy(scene.proj, projection.m.data(), sizeof(scene.proj));
  scene.lightDirAndAmbient[0] = 0.2f;
  scene.lightDirAndAmbient[1] = 1.0f;
  scene.lightDirAndAmbient[2] = 0.3f;
  scene.lightDirAndAmbient[3] = 0.25f;
  scene.cameraPos[0] = cameraPos.x;
  scene.cameraPos[1] = cameraPos.y;
  scene.cameraPos[2] = cameraPos.z;
  scene.cameraPos[3] = 1.0f;
  if (gConfig.fogEnabled) {
    scene.fogColor[0] = 0.08f;
    scene.fogColor[1] = 0.1f;
    scene.fogColor[2] = 0.16f;
    scene.fogColor[3] = 1.0f;
    scene.fogParams[0] = gConfig.fogDensity;
    scene.fogParams[1] = 0.25f;
    scene.fogParams[2] = -1.0f;
    scene.fogParams[3] = 0.8f;
    scene.fogParams2[0] = 3.0f;
    scene.fogParams2[1] = 0.35f;
    scene.fogParams2[2] = 0.0f;
    scene.fogParams2[3] = 0.0f;
  } else {
    scene.fogColor[0] = 0.0f;
    scene.fogColor[1] = 0.0f;
    scene.fogColor[2] = 0.0f;
    scene.fogColor[3] = 1.0f;
    scene.fogParams[0] = 0.0f;
    scene.fogParams[1] = 0.0f;
    scene.fogParams[2] = 0.0f;
    scene.fogParams[3] = 0.0f;
    scene.fogParams2[0] = 0.0f;
    scene.fogParams2[1] = 0.0f;
    scene.fogParams2[2] = 0.0f;
    scene.fogParams2[3] = 0.0f;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, gSceneUbo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneUniforms), &scene);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  if (gSkyboxShader && gSkyboxShader->IsValid() && gEnvironmentMap &&
      gConfig.skyboxEnabled) {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    gSkyboxShader->Use();
    gSkyboxShader->SetFloat("uScale", 60.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gEnvironmentMap);
    DrawMesh(gCubeMesh);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
  }

  if (gEnvironmentMap) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gEnvironmentMap);
  }

  if (sysMon) {
    DrawCPU((float)sysMon->GetCpuUsage());
    DrawRAM((float)sysMon->GetRamUsage());
    DrawDisk((float)sysMon->GetDiskUsage());
  }

  if (gParticles && sysMon && gConfig.particlesEnabled) {
    gParticles->Update(dtSeconds, *sysMon);
    gParticles->Draw();
  }

  if (postReady) {
    glDisable(GL_DEPTH_TEST);

    GLuint bloomTexture = 0;
    if (bloomReady) {
      // Bloom extraction pass.
      glBindFramebuffer(GL_FRAMEBUFFER, gPost.pingpongFbo[0]);
      gPost.extractShader->Use();
      gPost.extractShader->SetFloat("uThreshold", gConfig.bloomThreshold);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gPost.hdrColor);
      DrawFullScreenQuad(gPost);

      // Blur ping-pong passes.
      bool horizontal = true;
      bool firstIteration = true;
      const int blurPasses = 6;
      gPost.blurShader->Use();
      gPost.blurShader->SetVec2("uTexelSize", 1.0f / width, 1.0f / height);
      for (int i = 0; i < blurPasses; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER,
                          gPost.pingpongFbo[horizontal ? 1 : 0]);
        gPost.blurShader->SetInt("uHorizontal", horizontal ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,
                      firstIteration ? gPost.pingpongColor[0]
                                     : gPost.pingpongColor[horizontal ? 0 : 1]);
        DrawFullScreenQuad(gPost);
        horizontal = !horizontal;
        if (firstIteration) {
          firstIteration = false;
        }
      }

      bloomTexture = gPost.pingpongColor[horizontal ? 0 : 1];
    }

    if (fxaaReady) {
      glBindFramebuffer(GL_FRAMEBUFFER, gPost.ldrFbo);
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    gPost.tonemapShader->Use();
    gPost.tonemapShader->SetFloat("uExposure", gConfig.exposure);
    gPost.tonemapShader->SetFloat("uBloomStrength",
                                  bloomReady ? gConfig.bloomStrength : 0.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPost.hdrColor);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    DrawFullScreenQuad(gPost);

    if (fxaaReady) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      gPost.fxaaShader->Use();
      gPost.fxaaShader->SetVec2("uTexelSize", 1.0f / width, 1.0f / height);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gPost.ldrColor);
      DrawFullScreenQuad(gPost);
    }

    glEnable(GL_DEPTH_TEST);
  }

  SwapBuffers(hDC);
}

void CleanupOpenGL(HWND hwnd) {
  if (sysMon)
    delete sysMon;

  if (gParticles) {
    gParticles->Cleanup();
    delete gParticles;
    gParticles = nullptr;
  }

  if (gShader) {
    delete gShader;
    gShader = nullptr;
  }

  if (gSceneUbo) {
    glDeleteBuffers(1, &gSceneUbo);
    gSceneUbo = 0;
  }

  DestroyMesh(gCubeMesh);
  DestroyMesh(gSphereMesh);
  DestroyMesh(gRingMesh);

  DestroyPostProcessTargets(gPost);
  if (gPost.quadVbo) {
    glDeleteBuffers(1, &gPost.quadVbo);
    gPost.quadVbo = 0;
  }
  if (gPost.quadVao) {
    glDeleteVertexArrays(1, &gPost.quadVao);
    gPost.quadVao = 0;
  }
  if (gPost.extractShader) {
    delete gPost.extractShader;
    gPost.extractShader = nullptr;
  }
  if (gPost.blurShader) {
    delete gPost.blurShader;
    gPost.blurShader = nullptr;
  }
  if (gPost.tonemapShader) {
    delete gPost.tonemapShader;
    gPost.tonemapShader = nullptr;
  }
  if (gPost.fxaaShader) {
    delete gPost.fxaaShader;
    gPost.fxaaShader = nullptr;
  }

  if (gSkyboxShader) {
    delete gSkyboxShader;
    gSkyboxShader = nullptr;
  }

  if (gEnvironmentMap) {
    glDeleteTextures(1, &gEnvironmentMap);
    gEnvironmentMap = 0;
  }

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);
  ReleaseDC(hwnd, hDC);
}

void SetConfig(const Config &config) {
  gConfig = config;
  gPost.fxaaEnabled = (config.quality != QualityTier::Low);
  gPost.bloomEnabled = config.bloomEnabled;
}
