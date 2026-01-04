#include "glad.h"

static void *glad_get_proc(const char *name) {
  void *proc = (void *)wglGetProcAddress(name);
  if (proc) {
    return proc;
  }

  static HMODULE module = NULL;
  if (!module) {
    module = LoadLibraryA("opengl32.dll");
  }
  if (!module) {
    return NULL;
  }

  return (void *)GetProcAddress(module, name);
}

int gladLoadGLLoader(GLADloadproc load) {
  if (!load) {
    return 0;
  }

  if (!load("glGetString")) {
    return 0;
  }

  return 1;
}

int gladLoadGL(void) { return gladLoadGLLoader(glad_get_proc); }
