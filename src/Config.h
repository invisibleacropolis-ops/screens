#pragma once

#include <string>

enum class QualityTier {
  Low = 0,
  Medium = 1,
  High = 2,
};

struct Config {
  // Quality
  QualityTier quality = QualityTier::High;

  // Visual Effects
  bool bloomEnabled = true;
  float bloomThreshold = 0.7f;
  float bloomStrength = 0.8f;
  float exposure = 1.0f;
  bool fxaaEnabled = true;

  // Fog
  bool fogEnabled = true;
  float fogDensity = 0.045f;

  // Particles
  bool particlesEnabled = true;
  int particleCount = 6000; // Overrides quality-based default if set

  // Scene
  float rotationSpeed = 0.2f;

  // Camera
  float cameraDistance = 12.0f;
  float cameraHeight = 5.0f;
  float fieldOfView = 45.0f;

  // Background
  bool skyboxEnabled = true;
  int bgColorR = 0;
  int bgColorG = 13;
  int bgColorB = 25;
};

Config LoadConfig();
bool SaveConfig(const Config &config);
int GetParticleCount(const Config &config);
std::wstring GetConfigPath();
