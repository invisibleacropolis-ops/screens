#pragma once

#include <chrono>
#include <ctime>
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
    // Get executable path to store log next to it
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string fullPath(path);
    std::string dir = fullPath.substr(0, fullPath.find_last_of("\\/"));
    std::string logPath = dir + "\\screensaver_debug.log";

    m_logFile.open(logPath, std::ios::out | std::ios::trunc);
  }

  ~Logger() {
    if (m_logFile.is_open()) {
      m_logFile.close();
    }
  }

  std::ofstream m_logFile;
  std::mutex m_mutex;
};
