#pragma once

#include "graphics/PostProcessConfig.h"
#include <array>
#include <string>


enum class QualityTier {
  Low = 0,
  Medium = 1,
  High = 2,
};

// Mesh types available for metric visualizations
enum class MeshType {
  Sphere = 0,
  Cube = 1,
  Ring = 2,
  None = 3, // Disable visualization
};

// Configuration for a single metric visualization
struct MetricConfig {
  bool enabled = true;
  float threshold = 0.0f; // Value below which effect is minimal (0-100)
  float strength = 1.0f;  // Multiplier for the effect (0-2)
  MeshType meshType = MeshType::Sphere; // Which mesh to use
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
  int particleCount = 6000;

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

  // Metric Visualizations
  MetricConfig cpuMetric = {true, 0.0f, 1.0f, MeshType::Sphere};
  MetricConfig ramMetric = {true, 0.0f, 1.0f, MeshType::Cube};
  MetricConfig diskMetric = {true, 0.0f, 1.0f, MeshType::Ring};
  MetricConfig networkMetric = {true, 0.0f, 1.0f,
                                MeshType::None}; // None = particles only

  // Layer Configurations (Layout + FX)
  // Index 0=CPU, 1=RAM, 2=Disk, 3=Network
  std::array<PostProcessConfig, 4> layerConfigs;
};

Config LoadConfig();
bool SaveConfig(const Config &config);
int GetParticleCount(const Config &config);
std::wstring GetConfigPath();

// Helper to convert MeshType to/from string
const char *MeshTypeToString(MeshType type);
MeshType StringToMeshType(const std::string &str);
