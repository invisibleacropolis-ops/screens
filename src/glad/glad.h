#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <tchar.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*GLADloadproc)(const char *name);

int gladLoadGL(void);
int gladLoadGLLoader(GLADloadproc load);

/* ------------------------------------------------------------------------- */
/* OpenGL type definitions                                                   */
/* ------------------------------------------------------------------------- */

/* Base types from OpenGL specification */
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

/* ------------------------------------------------------------------------- */
/* OpenGL constants                                                          */
/* ------------------------------------------------------------------------- */

/* Boolean values */
#define GL_TRUE 1
#define GL_FALSE 0

/* Data types */
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_INT 0x1405

/* Draw modes */
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_TRIANGLES 0x0004

/* Clear bits */
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000

/* Texture parameters */
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_LINEAR 0x2601

/* Basic texture formats */
#define GL_RGB 0x1907
#define GL_RGBA 0x1908

/* Blend functions */
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE 0x0001
#define GL_ONE_MINUS_SRC_ALPHA 0x0303

/* Depth testing */
#define GL_DEPTH_TEST 0x0B71
#define GL_LESS 0x0201

/* Shader types */
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_INVALID_INDEX 0xFFFFFFFFu

/* Buffer objects */
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8

/* Texture formats */
#define GL_RED 0x1903
#define GL_R8 0x8229
#define GL_RGBA8 0x8058
#define GL_SRGB8 0x8C41
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_RGBA16F 0x881A
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_WRAP_R 0x8072

/* Cubemaps */
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A

/* Framebuffer objects */
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER_SRGB 0x8DB9

/* Texture units */
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1

/* Version query */
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C

/* Point sprites */
#define GL_PROGRAM_POINT_SIZE 0x8642

/* Depth function */
#define GL_LEQUAL 0x0203

/* ------------------------------------------------------------------------- */
/* OpenGL 1.1 function typedefs (from opengl32.dll)                          */
/* ------------------------------------------------------------------------- */

typedef void(APIENTRY *PFNGLCLEARPROC)(GLbitfield mask);
typedef void(APIENTRY *PFNGLCLEARCOLORPROC)(GLclampf red, GLclampf green,
                                            GLclampf blue, GLclampf alpha);
typedef void(APIENTRY *PFNGLENABLEPROC)(GLenum cap);
typedef void(APIENTRY *PFNGLDISABLEPROC)(GLenum cap);
typedef void(APIENTRY *PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width,
                                          GLsizei height);
typedef void(APIENTRY *PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first,
                                            GLsizei count);
typedef void(APIENTRY *PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count,
                                              GLenum type, const void *indices);
typedef void(APIENTRY *PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void(APIENTRY *PFNGLDELETETEXTURESPROC)(GLsizei n,
                                                const GLuint *textures);
typedef void(APIENTRY *PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void(APIENTRY *PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level,
                                            GLint internalformat, GLsizei width,
                                            GLsizei height, GLint border,
                                            GLenum format, GLenum type,
                                            const void *pixels);
typedef void(APIENTRY *PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname,
                                               GLint param);
typedef void(APIENTRY *PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void(APIENTRY *PFNGLDEPTHMASKPROC)(GLboolean flag);
typedef void(APIENTRY *PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void(APIENTRY *PFNGLGETINTEGERVPROC)(GLenum pname, GLint *data);

/* ------------------------------------------------------------------------- */
/* OpenGL extension function typedefs                                        */
/* ------------------------------------------------------------------------- */

/* Shader functions */
typedef GLuint(APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void(APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count,
                                              const GLchar *const *string,
                                              const GLint *length);
typedef void(APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void(APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname,
                                             GLint *params);
typedef void(APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader,
                                                  GLsizei bufSize,
                                                  GLsizei *length,
                                                  GLchar *infoLog);
typedef void(APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
typedef GLuint(APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void(APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void(APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void(APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname,
                                              GLint *params);
typedef void(APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program,
                                                   GLsizei bufSize,
                                                   GLsizei *length,
                                                   GLchar *infoLog);
typedef void(APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void(APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint(APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program,
                                                     const GLchar *name);
typedef void(APIENTRY *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void(APIENTRY *PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void(APIENTRY *PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0,
                                           GLfloat v1);
typedef void(APIENTRY *PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0,
                                           GLfloat v1, GLfloat v2);
typedef void(APIENTRY *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value);
typedef GLuint(APIENTRY *PFNGLGETUNIFORMBLOCKINDEXPROC)(
    GLuint program, const GLchar *uniformBlockName);
typedef void(APIENTRY *PFNGLUNIFORMBLOCKBINDINGPROC)(
    GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

/* VAO functions */
typedef void(APIENTRY *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void(APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n,
                                                    const GLuint *arrays);
typedef void(APIENTRY *PFNGLBINDVERTEXARRAYPROC)(GLuint array);

/* VBO functions */
typedef void(APIENTRY *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void(APIENTRY *PFNGLDELETEBUFFERSPROC)(GLsizei n,
                                               const GLuint *buffers);
typedef void(APIENTRY *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void(APIENTRY *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size,
                                            const void *data, GLenum usage);
typedef void(APIENTRY *PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset,
                                               GLsizeiptr size,
                                               const void *data);
typedef void(APIENTRY *PFNGLBINDBUFFERRANGEPROC)(GLenum target, GLuint index,
                                                 GLuint buffer, GLintptr offset,
                                                 GLsizeiptr size);

/* Vertex attrib functions */
typedef void(APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void(APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size,
                                                     GLenum type,
                                                     GLboolean normalized,
                                                     GLsizei stride,
                                                     const void *pointer);
typedef void(APIENTRY *PFNGLVERTEXATTRIBDIVISORPROC)(GLuint index,
                                                     GLuint divisor);

/* Draw functions */
typedef void(APIENTRY *PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first,
                                                     GLsizei count,
                                                     GLsizei instancecount);

/* Framebuffer functions */
typedef void(APIENTRY *PFNGLGENFRAMEBUFFERSPROC)(GLsizei n,
                                                 GLuint *framebuffers);
typedef void(APIENTRY *PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n,
                                                    const GLuint *framebuffers);
typedef void(APIENTRY *PFNGLBINDFRAMEBUFFERPROC)(GLenum target,
                                                 GLuint framebuffer);
typedef void(APIENTRY *PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target,
                                                      GLenum attachment,
                                                      GLenum textarget,
                                                      GLuint texture,
                                                      GLint level);
typedef void(APIENTRY *PFNGLFRAMEBUFFERRENDERBUFFERPROC)(
    GLenum target, GLenum attachment, GLenum renderbuffertarget,
    GLuint renderbuffer);

/* Renderbuffer functions */
typedef void(APIENTRY *PFNGLGENRENDERBUFFERSPROC)(GLsizei n,
                                                  GLuint *renderbuffers);
typedef void(APIENTRY *PFNGLDELETERENDERBUFFERSPROC)(
    GLsizei n, const GLuint *renderbuffers);
typedef void(APIENTRY *PFNGLBINDRENDERBUFFERPROC)(GLenum target,
                                                  GLuint renderbuffer);
typedef void(APIENTRY *PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target,
                                                     GLenum internalformat,
                                                     GLsizei width,
                                                     GLsizei height);

/* Texture functions */
typedef void(APIENTRY *PFNGLACTIVETEXTUREPROC)(GLenum texture);

/* ------------------------------------------------------------------------- */
/* OpenGL 1.1 function declarations (use opengl32.dll directly)              */
/* ------------------------------------------------------------------------- */

/* Use WINGDIAPI for OpenGL 1.1 functions from opengl32.dll */
#ifndef WINGDIAPI
#define WINGDIAPI __declspec(dllimport)
#endif

WINGDIAPI void APIENTRY glClear(GLbitfield mask);
WINGDIAPI void APIENTRY glClearColor(GLclampf red, GLclampf green,
                                     GLclampf blue, GLclampf alpha);
WINGDIAPI void APIENTRY glEnable(GLenum cap);
WINGDIAPI void APIENTRY glDisable(GLenum cap);
WINGDIAPI void APIENTRY glViewport(GLint x, GLint y, GLsizei width,
                                   GLsizei height);
WINGDIAPI void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count);
WINGDIAPI void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type,
                                       const void *indices);
WINGDIAPI void APIENTRY glGenTextures(GLsizei n, GLuint *textures);
WINGDIAPI void APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures);
WINGDIAPI void APIENTRY glBindTexture(GLenum target, GLuint texture);
WINGDIAPI void APIENTRY glTexImage2D(GLenum target, GLint level,
                                     GLint internalformat, GLsizei width,
                                     GLsizei height, GLint border,
                                     GLenum format, GLenum type,
                                     const void *pixels);
WINGDIAPI void APIENTRY glTexParameteri(GLenum target, GLenum pname,
                                        GLint param);
WINGDIAPI void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor);
WINGDIAPI void APIENTRY glDepthMask(GLboolean flag);
WINGDIAPI void APIENTRY glDepthFunc(GLenum func);
WINGDIAPI void APIENTRY glGetIntegerv(GLenum pname, GLint *data);

/* ------------------------------------------------------------------------- */
/* OpenGL extension function declarations                                    */
/* ------------------------------------------------------------------------- */

/* Shader functions */
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;

/* VAO functions */
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

/* VBO functions */
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLBINDBUFFERRANGEPROC glBindBufferRange;

/* Vertex attrib functions */
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

/* Draw functions */
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;

/* Framebuffer functions */
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

/* Renderbuffer functions */
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;

/* Texture functions */
extern PFNGLACTIVETEXTUREPROC glActiveTexture;

#ifdef __cplusplus
}
#endif
