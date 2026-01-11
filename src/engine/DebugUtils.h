#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "../Logger.h"
#include "../glad/glad.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

inline void CheckGLError(const char *label) {
  GLenum err;
  while ((err = glGetError()) != 0) {
    std::stringstream ss;
    ss << "GL Error [" << label << "]: 0x" << std::hex << err;
    Logger::LogS(ss.str());
  }
}

inline void LogMatrix(const char *name, const float *m) {
  std::stringstream ss;
  ss << name << ":\n";
  for (int i = 0; i < 4; ++i) {
    ss << "  [ ";
    for (int j = 0; j < 4; ++j)
      ss << m[j * 4 + i] << " ";
    ss << "]\n";
  }
  Logger::LogS(ss.str());
}

#endif // DEBUG_UTILS_H
