#include "Particles.h"

#include <algorithm>
#include <cmath>

#include "SystemMonitor.h"
#include "graphics/Shader.h"


namespace {
constexpr float kTwoPi = 6.28318530718f;
constexpr float kMinLife = 1.0f;
constexpr float kMaxLife = 3.5f;
} // namespace

Particles::Particles(std::size_t maxParticles)
    : maxParticles_(maxParticles), particles_(maxParticles),
      instances_(maxParticles), rng_(std::random_device{}()),
      uniform01_(0.0f, 1.0f) {}

Particles::~Particles() { Cleanup(); }

bool Particles::Initialize() {
  GLint major = 0;
  GLint minor = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  gpuSimulationAvailable_ = (major > 4) || (major == 4 && minor >= 3);

  shader_ = new Shader("assets/shaders/particles.vert",
                       "assets/shaders/particles.frag");
  if (!shader_ || !shader_->IsValid()) {
    return false;
  }

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(1, &baseVbo_);
  glBindBuffer(GL_ARRAY_BUFFER, baseVbo_);
  const float baseVertex[3] = {0.0f, 0.0f, 0.0f};
  glBufferData(GL_ARRAY_BUFFER, sizeof(baseVertex), baseVertex, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(baseVertex),
                        reinterpret_cast<void *>(0));

  glGenBuffers(1, &instanceVbo_);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
  glBufferData(GL_ARRAY_BUFFER, maxParticles_ * sizeof(InstanceData), nullptr,
               GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
      reinterpret_cast<void *>(offsetof(InstanceData, position)));
  glVertexAttribDivisor(1, 1);

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
      reinterpret_cast<void *>(offsetof(InstanceData, color)));
  glVertexAttribDivisor(2, 1);

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                        reinterpret_cast<void *>(offsetof(InstanceData, size)));
  glVertexAttribDivisor(3, 1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  shader_->Use();
  shader_->BindUniformBlock("SceneData", 0);

  return true;
}

void Particles::Cleanup() {
  if (instanceVbo_) {
    glDeleteBuffers(1, &instanceVbo_);
    instanceVbo_ = 0;
  }
  if (baseVbo_) {
    glDeleteBuffers(1, &baseVbo_);
    baseVbo_ = 0;
  }
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
    vao_ = 0;
  }
  if (shader_) {
    delete shader_;
    shader_ = nullptr;
  }
}

float Particles::SmoothValue(float current, float target, float dtSeconds,
                             float riseRate, float fallRate) const {
  if (dtSeconds <= 0.0f) {
    return current;
  }
  const float rate = (target > current) ? riseRate : fallRate;
  const float t = 1.0f - std::exp(-rate * dtSeconds);
  return current + (target - current) * t;
}

float Particles::RandomRange(float minValue, float maxValue) {
  return minValue + (maxValue - minValue) * uniform01_(rng_);
}

void Particles::SpawnParticle(const SmoothedMetrics &metrics) {
  if (liveCount_ >= maxParticles_) {
    return;
  }

  Particle &particle = particles_[liveCount_++];

  const float theta = RandomRange(0.0f, kTwoPi);
  const float phi = std::acos(1.0f - 2.0f * uniform01_(rng_));
  const float radius = RandomRange(0.5f, 2.0f + metrics.cpu * 1.2f);

  particle.position[0] = std::cos(theta) * std::sin(phi) * radius;
  particle.position[1] = std::cos(phi) * radius * 0.6f;
  particle.position[2] = std::sin(theta) * std::sin(phi) * radius;

  const float speed = 0.5f + metrics.disk * 3.0f;
  const float verticalBias = 0.5f + metrics.cpu * 0.8f;
  particle.velocity[0] = RandomRange(-1.0f, 1.0f) * speed;
  particle.velocity[1] = (RandomRange(0.2f, 1.0f) + verticalBias) * speed;
  particle.velocity[2] = RandomRange(-1.0f, 1.0f) * speed;

  const float netPulse = 0.2f + metrics.net * 0.8f;
  particle.color[0] = 0.2f + metrics.ram * 0.8f;
  particle.color[1] = 0.35f + (1.0f - metrics.ram) * 0.4f;
  particle.color[2] = 0.6f + metrics.cpu * 0.4f;
  particle.color[3] = netPulse;

  particle.size = 2.0f + metrics.cpu * 6.0f + metrics.net * 4.0f;
  particle.life = RandomRange(kMinLife, kMaxLife);
}

void Particles::Update(float dtSeconds, const SystemMonitor &monitor) {
  const float cpuTarget = static_cast<float>(monitor.GetCpuUsage() / 100.0);
  const float ramTarget = static_cast<float>(monitor.GetRamUsage() / 100.0);
  const float diskTarget = static_cast<float>(monitor.GetDiskUsage() / 100.0);
  const float netTarget =
      static_cast<float>(monitor.GetNetworkBytesPerSec() / (1024.0 * 1024.0));

  smoothed_.cpu = SmoothValue(smoothed_.cpu, cpuTarget, dtSeconds, 2.5f, 1.5f);
  smoothed_.ram = SmoothValue(smoothed_.ram, ramTarget, dtSeconds, 2.0f, 1.2f);
  smoothed_.disk =
      SmoothValue(smoothed_.disk, diskTarget, dtSeconds, 2.0f, 1.0f);
  smoothed_.net = SmoothValue(smoothed_.net, std::min(netTarget, 1.0f),
                              dtSeconds, 3.0f, 1.5f);

  const float spawnRate =
      40.0f + smoothed_.cpu * 200.0f + smoothed_.net * 80.0f;
  spawnAccumulator_ += spawnRate * dtSeconds;

  while (spawnAccumulator_ >= 1.0f) {
    SpawnParticle(smoothed_);
    spawnAccumulator_ -= 1.0f;
  }

  for (std::size_t i = 0; i < liveCount_;) {
    Particle &particle = particles_[i];
    particle.life -= dtSeconds;

    if (particle.life <= 0.0f) {
      particles_[i] = particles_[liveCount_ - 1];
      --liveCount_;
      continue;
    }

    particle.velocity[1] += 0.4f * dtSeconds;
    particle.position[0] += particle.velocity[0] * dtSeconds;
    particle.position[1] += particle.velocity[1] * dtSeconds;
    particle.position[2] += particle.velocity[2] * dtSeconds;

    const float fade = std::clamp(particle.life / kMaxLife, 0.0f, 1.0f);
    particle.color[3] = fade;

    ++i;
  }

  if (gpuSimulationAvailable_) {
    // Placeholder for future GPU-driven simulation using compute or
    // transform feedback when OpenGL 4.3+ is available.
  }
}

void Particles::Draw() {
  if (!shader_ || !shader_->IsValid() || liveCount_ == 0 || !vao_) {
    return;
  }

  for (std::size_t i = 0; i < liveCount_; ++i) {
    const Particle &particle = particles_[i];
    InstanceData &instance = instances_[i];
    instance.position[0] = particle.position[0];
    instance.position[1] = particle.position[1];
    instance.position[2] = particle.position[2];
    instance.color[0] = particle.color[0];
    instance.color[1] = particle.color[1];
    instance.color[2] = particle.color[2];
    instance.color[3] = particle.color[3];
    instance.size = particle.size;
  }

  glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, liveCount_ * sizeof(InstanceData),
                  instances_.data());

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glDepthMask(GL_FALSE);
  glEnable(GL_PROGRAM_POINT_SIZE);

  shader_->Use();
  glBindVertexArray(vao_);
  glDrawArraysInstanced(GL_POINTS, 0, 1, static_cast<GLsizei>(liveCount_));

  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}
