#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <windows.h>

class Logger {
public:
  static Logger &Instance() {
    static Logger instance;
    return instance;
  }

  void Log(const std::string &message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
      // Timestamp
      auto now = std::chrono::system_clock::now();
      auto time = std::chrono::system_clock::to_time_t(now);
      struct tm tm;
      localtime_s(&tm, &time);

      m_logFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
                << message << std::endl;
      m_logFile.flush(); // Flush immediately
    }
  }

  static void LogS(const std::string &message) { Instance().Log(message); }

private:
  Logger() {
    // Store logs under SSOT output root: <exe dir>/.out/logs/
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::filesystem::path exePath(path);
    std::filesystem::path logDir = exePath.parent_path() / ".out" / "logs";
    std::error_code ec;
    std::filesystem::create_directories(logDir, ec);
    std::filesystem::path logPath = logDir / "screensaver_debug.log";

    m_logFile.open(logPath.string(), std::ios::out | std::ios::trunc);
  }

  ~Logger() {
    if (m_logFile.is_open()) {
      m_logFile.close();
    }
  }

  std::ofstream m_logFile;
  std::mutex m_mutex;
};
