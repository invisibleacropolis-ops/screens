#include "glad.h"

/* ------------------------------------------------------------------------- */
/* Function pointer definitions                                              */
/* ------------------------------------------------------------------------- */

/* Shader functions */
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM2FPROC glUniform2f = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = NULL;

/* VAO functions */
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;

/* VBO functions */
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = NULL;

/* Vertex attrib functions */
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = NULL;

/* Draw functions */
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = NULL;

/* Framebuffer functions */
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;

/* Renderbuffer functions */
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;

/* Texture functions */
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;

/* ------------------------------------------------------------------------- */
/* Internal helpers                                                          */
/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

int gladLoadGLLoader(GLADloadproc load) {
  if (!load) {
    return 0;
  }

  /* Shader functions */
  glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
  glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
  glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
  glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
  glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
  glGetUniformLocation =
      (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
  glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
  glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
  glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
  glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
  glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
  glGetUniformBlockIndex =
      (PFNGLGETUNIFORMBLOCKINDEXPROC)load("glGetUniformBlockIndex");
  glUniformBlockBinding =
      (PFNGLUNIFORMBLOCKBINDINGPROC)load("glUniformBlockBinding");

  /* VAO functions */
  glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
  glDeleteVertexArrays =
      (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
  glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");

  /* VBO functions */
  glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
  glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
  glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
  glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
  glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
  glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");

  /* Vertex attrib functions */
  glEnableVertexAttribArray =
      (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
  glVertexAttribPointer =
      (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
  glVertexAttribDivisor =
      (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");

  /* Draw functions */
  glDrawArraysInstanced =
      (PFNGLDRAWARRAYSINSTANCEDPROC)load("glDrawArraysInstanced");

  /* Framebuffer functions */
  glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
  glDeleteFramebuffers =
      (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
  glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
  glFramebufferTexture2D =
      (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
  glFramebufferRenderbuffer =
      (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load("glFramebufferRenderbuffer");

  /* Renderbuffer functions */
  glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)load("glGenRenderbuffers");
  glDeleteRenderbuffers =
      (PFNGLDELETERENDERBUFFERSPROC)load("glDeleteRenderbuffers");
  glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)load("glBindRenderbuffer");
  glRenderbufferStorage =
      (PFNGLRENDERBUFFERSTORAGEPROC)load("glRenderbufferStorage");

  /* Texture functions */
  glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");

  return 1;
}

int gladLoadGL(void) { return gladLoadGLLoader(glad_get_proc); }
