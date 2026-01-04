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
  std::transform(
      value.begin(), value.end(), value.begin(),
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

int ParseInt(const std::string &value, int defaultValue) {
  try {
    return std::stoi(Trim(value));
  } catch (...) {
    return defaultValue;
  }
}

float ParseFloat(const std::string &value, float defaultValue) {
  try {
    return std::stof(Trim(value));
  } catch (...) {
    return defaultValue;
  }
}

QualityTier ParseQuality(const std::string &value, QualityTier fallback) {
  const std::string normalized = ToLower(Trim(value));
  if (normalized == "low" || normalized == "0") {
    return QualityTier::Low;
  }
  if (normalized == "medium" || normalized == "1") {
    return QualityTier::Medium;
  }
  if (normalized == "high" || normalized == "2") {
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

// MeshType conversion functions (public)
const char *MeshTypeToString(MeshType type) {
  switch (type) {
  case MeshType::Sphere:
    return "sphere";
  case MeshType::Cube:
    return "cube";
  case MeshType::Ring:
    return "ring";
  case MeshType::None:
    return "none";
  }
  return "sphere";
}

MeshType StringToMeshType(const std::string &str) {
  std::string lower = str;
  std::transform(
      lower.begin(), lower.end(), lower.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (lower == "sphere" || lower == "0")
    return MeshType::Sphere;
  if (lower == "cube" || lower == "1")
    return MeshType::Cube;
  if (lower == "ring" || lower == "2")
    return MeshType::Ring;
  if (lower == "none" || lower == "3")
    return MeshType::None;
  return MeshType::Sphere;
}

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

    // Quality
    if (key == "quality") {
      config.quality = ParseQuality(value, config.quality);
    }
    // Visual Effects
    else if (key == "bloom") {
      config.bloomEnabled = ParseBool(value, config.bloomEnabled);
    } else if (key == "bloom_threshold") {
      config.bloomThreshold = ParseFloat(value, config.bloomThreshold);
    } else if (key == "bloom_strength") {
      config.bloomStrength = ParseFloat(value, config.bloomStrength);
    } else if (key == "exposure") {
      config.exposure = ParseFloat(value, config.exposure);
    } else if (key == "fxaa") {
      config.fxaaEnabled = ParseBool(value, config.fxaaEnabled);
    }
    // Fog
    else if (key == "fog") {
      config.fogEnabled = ParseBool(value, config.fogEnabled);
    } else if (key == "fog_density") {
      config.fogDensity = ParseFloat(value, config.fogDensity);
    }
    // Particles
    else if (key == "particles") {
      config.particlesEnabled = ParseBool(value, config.particlesEnabled);
    } else if (key == "particle_count") {
      config.particleCount = ParseInt(value, config.particleCount);
    }
    // Scene
    else if (key == "rotation_speed") {
      config.rotationSpeed = ParseFloat(value, config.rotationSpeed);
    }
    // Camera
    else if (key == "camera_distance") {
      config.cameraDistance = ParseFloat(value, config.cameraDistance);
    } else if (key == "camera_height") {
      config.cameraHeight = ParseFloat(value, config.cameraHeight);
    } else if (key == "field_of_view") {
      config.fieldOfView = ParseFloat(value, config.fieldOfView);
    }
    // Background
    else if (key == "skybox") {
      config.skyboxEnabled = ParseBool(value, config.skyboxEnabled);
    } else if (key == "bg_color_r") {
      config.bgColorR = std::clamp(ParseInt(value, config.bgColorR), 0, 255);
    } else if (key == "bg_color_g") {
      config.bgColorG = std::clamp(ParseInt(value, config.bgColorG), 0, 255);
    } else if (key == "bg_color_b") {
      config.bgColorB = std::clamp(ParseInt(value, config.bgColorB), 0, 255);
    }
    // CPU Metric
    else if (key == "cpu_enabled") {
      config.cpuMetric.enabled = ParseBool(value, config.cpuMetric.enabled);
    } else if (key == "cpu_threshold") {
      config.cpuMetric.threshold =
          ParseFloat(value, config.cpuMetric.threshold);
    } else if (key == "cpu_strength") {
      config.cpuMetric.strength = ParseFloat(value, config.cpuMetric.strength);
    } else if (key == "cpu_mesh") {
      config.cpuMetric.meshType = StringToMeshType(value);
    }
    // RAM Metric
    else if (key == "ram_enabled") {
      config.ramMetric.enabled = ParseBool(value, config.ramMetric.enabled);
    } else if (key == "ram_threshold") {
      config.ramMetric.threshold =
          ParseFloat(value, config.ramMetric.threshold);
    } else if (key == "ram_strength") {
      config.ramMetric.strength = ParseFloat(value, config.ramMetric.strength);
    } else if (key == "ram_mesh") {
      config.ramMetric.meshType = StringToMeshType(value);
    }
    // Disk Metric
    else if (key == "disk_enabled") {
      config.diskMetric.enabled = ParseBool(value, config.diskMetric.enabled);
    } else if (key == "disk_threshold") {
      config.diskMetric.threshold =
          ParseFloat(value, config.diskMetric.threshold);
    } else if (key == "disk_strength") {
      config.diskMetric.strength =
          ParseFloat(value, config.diskMetric.strength);
    } else if (key == "disk_mesh") {
      config.diskMetric.meshType = StringToMeshType(value);
    }
    // Network Metric
    else if (key == "network_enabled") {
      config.networkMetric.enabled =
          ParseBool(value, config.networkMetric.enabled);
    } else if (key == "network_threshold") {
      config.networkMetric.threshold =
          ParseFloat(value, config.networkMetric.threshold);
    } else if (key == "network_strength") {
      config.networkMetric.strength =
          ParseFloat(value, config.networkMetric.strength);
    } else if (key == "network_mesh") {
      config.networkMetric.meshType = StringToMeshType(value);
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

  file << "# OpenGL Screensaver Configuration\n\n";

  file << "# Quality: low, medium, high\n";
  file << "quality=" << QualityToString(config.quality) << "\n\n";

  file << "# Visual Effects\n";
  file << "bloom=" << (config.bloomEnabled ? "true" : "false") << "\n";
  file << "bloom_threshold=" << config.bloomThreshold << "\n";
  file << "bloom_strength=" << config.bloomStrength << "\n";
  file << "exposure=" << config.exposure << "\n";
  file << "fxaa=" << (config.fxaaEnabled ? "true" : "false") << "\n\n";

  file << "# Fog\n";
  file << "fog=" << (config.fogEnabled ? "true" : "false") << "\n";
  file << "fog_density=" << config.fogDensity << "\n\n";

  file << "# Particles\n";
  file << "particles=" << (config.particlesEnabled ? "true" : "false") << "\n";
  file << "particle_count=" << config.particleCount << "\n\n";

  file << "# Scene\n";
  file << "rotation_speed=" << config.rotationSpeed << "\n\n";

  file << "# Camera\n";
  file << "camera_distance=" << config.cameraDistance << "\n";
  file << "camera_height=" << config.cameraHeight << "\n";
  file << "field_of_view=" << config.fieldOfView << "\n\n";

  file << "# Background\n";
  file << "skybox=" << (config.skyboxEnabled ? "true" : "false") << "\n";
  file << "bg_color_r=" << config.bgColorR << "\n";
  file << "bg_color_g=" << config.bgColorG << "\n";
  file << "bg_color_b=" << config.bgColorB << "\n\n";

  // Metric Visualizations
  file << "# CPU Metric (mesh: sphere, cube, ring, none)\n";
  file << "cpu_enabled=" << (config.cpuMetric.enabled ? "true" : "false")
       << "\n";
  file << "cpu_threshold=" << config.cpuMetric.threshold << "\n";
  file << "cpu_strength=" << config.cpuMetric.strength << "\n";
  file << "cpu_mesh=" << MeshTypeToString(config.cpuMetric.meshType) << "\n\n";

  file << "# RAM Metric\n";
  file << "ram_enabled=" << (config.ramMetric.enabled ? "true" : "false")
       << "\n";
  file << "ram_threshold=" << config.ramMetric.threshold << "\n";
  file << "ram_strength=" << config.ramMetric.strength << "\n";
  file << "ram_mesh=" << MeshTypeToString(config.ramMetric.meshType) << "\n\n";

  file << "# Disk Metric\n";
  file << "disk_enabled=" << (config.diskMetric.enabled ? "true" : "false")
       << "\n";
  file << "disk_threshold=" << config.diskMetric.threshold << "\n";
  file << "disk_strength=" << config.diskMetric.strength << "\n";
  file << "disk_mesh=" << MeshTypeToString(config.diskMetric.meshType)
       << "\n\n";

  file << "# Network Metric\n";
  file << "network_enabled="
       << (config.networkMetric.enabled ? "true" : "false") << "\n";
  file << "network_threshold=" << config.networkMetric.threshold << "\n";
  file << "network_strength=" << config.networkMetric.strength << "\n";
  file << "network_mesh=" << MeshTypeToString(config.networkMetric.meshType)
       << "\n";

  return true;
}

int GetParticleCount(const Config &config) {
  if (!config.particlesEnabled) {
    return 0;
  }

  // If user explicitly set particle count, use it
  if (config.particleCount > 0) {
    return config.particleCount;
  }

  // Otherwise base on quality tier
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
