#pragma once

#include <cstddef>
#include <random>
#include <vector>

#include "glad/glad.h"

class Shader;
class SystemMonitor;

// GPU-friendly particle system that uses instanced rendering.
//
// Particles are simulated on the CPU for now, with smoothed/decayed inputs
// from SystemMonitor to prevent jitter from spiky telemetry values. The
// instancing path makes it easy to swap in GPU simulation later.
class Particles {
public:
  explicit Particles(std::size_t maxParticles);
  ~Particles();

  bool Initialize();
  void Update(float dtSeconds, const SystemMonitor &monitor);
  void Draw() const;
  void Cleanup();

private:
  struct Particle {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {0.0f, 0.0f, 0.0f};
    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float size = 1.0f;
    float life = 0.0f;
  };

  struct InstanceData {
    float position[3];
    float color[4];
    float size;
    float pad[2];
  };

  struct SmoothedMetrics {
    float cpu = 0.0f;
    float ram = 0.0f;
    float disk = 0.0f;
    float net = 0.0f;
  };

  float SmoothValue(float current, float target, float dtSeconds,
                    float riseRate, float fallRate) const;
  float RandomRange(float minValue, float maxValue);
  void SpawnParticle(const SmoothedMetrics &metrics);

  std::size_t maxParticles_ = 0;
  std::size_t liveCount_ = 0;
  std::vector<Particle> particles_;
  std::vector<InstanceData> instances_;
  std::mt19937 rng_;
  std::uniform_real_distribution<float> uniform01_;
  float spawnAccumulator_ = 0.0f;
  SmoothedMetrics smoothed_;
  bool gpuSimulationAvailable_ = false;

  GLuint vao_ = 0;
  GLuint baseVbo_ = 0;
  GLuint instanceVbo_ = 0;
  Shader *shader_ = nullptr;
};
