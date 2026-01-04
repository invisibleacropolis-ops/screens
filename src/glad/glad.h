#pragma once

#include <windows.h>
#include <gl/GL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*GLADloadproc)(const char *name);

int gladLoadGL(void);
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif
