#pragma once

#include <string>

enum class QualityTier {
  Low = 0,
  Medium = 1,
  High = 2,
};

struct Config {
  QualityTier quality = QualityTier::High;
  bool bloomEnabled = true;
  bool fogEnabled = true;
  bool particlesEnabled = true;
};

Config LoadConfig();
bool SaveConfig(const Config &config);
int GetParticleCount(const Config &config);
std::wstring GetConfigPath();
