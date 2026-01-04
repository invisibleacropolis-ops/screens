// Test utilities for diagnosing rendering issues
#pragma once

#include "glad/glad.h"
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>


namespace DiagTest {

// Log to both OutputDebugString and a file
inline void Log(const std::string &message) {
  OutputDebugStringA(message.c_str());

  // Also log to file for easy viewing
  static bool initialized = false;
  static std::string logPath;

  if (!initialized) {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    logPath = exePath;
    size_t lastSlash = logPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
      logPath = logPath.substr(0, lastSlash + 1);
    }
    logPath += "screensaver_debug.log";
    initialized = true;

    // Clear old log
    std::ofstream clear(logPath, std::ios::trunc);
    clear.close();
  }

  std::ofstream log(logPath, std::ios::app);
  if (log.is_open()) {
    log << message;
    log.flush();
  }
}

// Check if a file exists
inline bool FileExists(const std::string &path) {
  std::ifstream f(path);
  return f.good();
}

// Get current working directory
inline std::string GetCwd() {
  char buffer[MAX_PATH];
  GetCurrentDirectoryA(MAX_PATH, buffer);
  return std::string(buffer);
}

// Get executable directory
inline std::string GetExeDir() {
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  std::string path(buffer);
  size_t lastSlash = path.find_last_of("\\/");
  if (lastSlash != std::string::npos) {
    return path.substr(0, lastSlash);
  }
  return path;
}

// Check GL function pointers
inline void DiagnoseGLFunctions() {
  Log("=== OpenGL Function Pointer Diagnostic ===\n");

  Log("glCreateShader: " + std::string(glCreateShader ? "LOADED" : "NULL") +
      "\n");
  Log("glShaderSource: " + std::string(glShaderSource ? "LOADED" : "NULL") +
      "\n");
  Log("glCompileShader: " + std::string(glCompileShader ? "LOADED" : "NULL") +
      "\n");
  Log("glCreateProgram: " + std::string(glCreateProgram ? "LOADED" : "NULL") +
      "\n");
  Log("glLinkProgram: " + std::string(glLinkProgram ? "LOADED" : "NULL") +
      "\n");
  Log("glUseProgram: " + std::string(glUseProgram ? "LOADED" : "NULL") + "\n");
  Log("glGenVertexArrays: " +
      std::string(glGenVertexArrays ? "LOADED" : "NULL") + "\n");
  Log("glBindVertexArray: " +
      std::string(glBindVertexArray ? "LOADED" : "NULL") + "\n");
  Log("glGenBuffers: " + std::string(glGenBuffers ? "LOADED" : "NULL") + "\n");
  Log("glBindBuffer: " + std::string(glBindBuffer ? "LOADED" : "NULL") + "\n");
  Log("glBufferData: " + std::string(glBufferData ? "LOADED" : "NULL") + "\n");
  Log("glActiveTexture: " + std::string(glActiveTexture ? "LOADED" : "NULL") +
      "\n");
  Log("glGenFramebuffers: " +
      std::string(glGenFramebuffers ? "LOADED" : "NULL") + "\n");
  Log("glBindFramebuffer: " +
      std::string(glBindFramebuffer ? "LOADED" : "NULL") + "\n");
  Log("glUniformMatrix4fv: " +
      std::string(glUniformMatrix4fv ? "LOADED" : "NULL") + "\n");

  Log("===========================================\n");
}

// Check shader file accessibility
inline void DiagnoseShaderFiles() {
  Log("=== Shader File Diagnostic ===\n");
  Log("Current working directory: " + GetCwd() + "\n");
  Log("Executable directory: " + GetExeDir() + "\n");

  const char *shaderPaths[] = {
      "assets/shaders/basic.vert",       "assets/shaders/basic.frag",
      "assets/shaders/environment.vert", "assets/shaders/environment.frag",
      "assets/shaders/particles.vert",   "assets/shaders/particles.frag"};

  for (const char *path : shaderPaths) {
    bool exists = FileExists(path);
    Log(std::string(path) + ": " + (exists ? "FOUND" : "NOT FOUND") + "\n");

    // Try from exe directory
    if (!exists) {
      std::string altPath = GetExeDir() + "/" + path;
      bool altExists = FileExists(altPath);
      Log("  (trying " + altPath + "): " + (altExists ? "FOUND" : "NOT FOUND") +
          "\n");
    }
  }

  Log("===============================\n");
}

// Run all diagnostics
inline void RunAllDiagnostics() {
  Log("\n\n========== SCREENSAVER DIAGNOSTIC START ==========\n");
  Log("Timestamp: " + std::to_string(GetTickCount()) + " ms\n\n");
  DiagnoseShaderFiles();
  DiagnoseGLFunctions();
  Log("========== SCREENSAVER DIAGNOSTIC END ==========\n\n");
}

} // namespace DiagTest
