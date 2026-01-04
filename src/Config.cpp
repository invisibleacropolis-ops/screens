#include "Config.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <windows.h>

namespace {
std::string Trim(const std::string &input) {
  const auto first = input.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return std::string();
  }
  const auto last = input.find_last_not_of(" \t\r\n");
  return input.substr(first, last - first + 1);
}

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

bool ParseBool(const std::string &value, bool defaultValue) {
  const std::string normalized = ToLower(Trim(value));
  if (normalized == "1" || normalized == "true" || normalized == "yes" ||
      normalized == "on") {
    return true;
  }
  if (normalized == "0" || normalized == "false" || normalized == "no" ||
      normalized == "off") {
    return false;
  }
  return defaultValue;
}

QualityTier ParseQuality(const std::string &value, QualityTier fallback) {
  const std::string normalized = ToLower(Trim(value));
  if (normalized == "low") {
    return QualityTier::Low;
  }
  if (normalized == "medium") {
    return QualityTier::Medium;
  }
  if (normalized == "high") {
    return QualityTier::High;
  }
  return fallback;
}

std::string QualityToString(QualityTier tier) {
  switch (tier) {
  case QualityTier::Low:
    return "low";
  case QualityTier::Medium:
    return "medium";
  case QualityTier::High:
    return "high";
  }
  return "high";
}
} // namespace

std::wstring GetConfigPath() {
  wchar_t modulePath[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
  std::filesystem::path path(modulePath);
  path = path.parent_path() / L"screensaver_config.ini";
  return path.wstring();
}

Config LoadConfig() {
  Config config;
  const std::filesystem::path path(GetConfigPath());
  std::ifstream file(path);
  if (!file.is_open()) {
    return config;
  }

  std::string line;
  while (std::getline(file, line)) {
    const std::string trimmed = Trim(line);
    if (trimmed.empty() || trimmed[0] == '#') {
      continue;
    }

    const auto separator = trimmed.find('=');
    if (separator == std::string::npos) {
      continue;
    }

    const std::string key = ToLower(Trim(trimmed.substr(0, separator)));
    const std::string value = Trim(trimmed.substr(separator + 1));

    if (key == "quality") {
      config.quality = ParseQuality(value, config.quality);
    } else if (key == "bloom") {
      config.bloomEnabled = ParseBool(value, config.bloomEnabled);
    } else if (key == "fog") {
      config.fogEnabled = ParseBool(value, config.fogEnabled);
    } else if (key == "particles") {
      config.particlesEnabled = ParseBool(value, config.particlesEnabled);
    }
  }

  return config;
}

bool SaveConfig(const Config &config) {
  const std::filesystem::path path(GetConfigPath());
  std::ofstream file(path, std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }

  file << "# Simple OpenGL Screensaver configuration\n";
  file << "quality=" << QualityToString(config.quality) << "\n";
  file << "bloom=" << (config.bloomEnabled ? "true" : "false") << "\n";
  file << "fog=" << (config.fogEnabled ? "true" : "false") << "\n";
  file << "particles=" << (config.particlesEnabled ? "true" : "false") << "\n";

  return true;
}

int GetParticleCount(const Config &config) {
  if (!config.particlesEnabled) {
    return 0;
  }

  switch (config.quality) {
  case QualityTier::Low:
    return 1000;
  case QualityTier::Medium:
    return 3000;
  case QualityTier::High:
    return 6000;
  }
  return 3000;
}
