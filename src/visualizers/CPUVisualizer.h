#pragma once

#include "IVisualizer.h"
#include <cmath>

// CPU Visualization: A pulsing shape that responds to CPU usage
class CPUVisualizer : public IVisualizer {
public:
  CPUVisualizer(const Config &config, Mesh &sphereMesh, Mesh &cubeMesh,
                Mesh &ringMesh)
      : m_config(config), m_sphereMesh(sphereMesh), m_cubeMesh(cubeMesh),
        m_ringMesh(ringMesh) {}

  void Init() override {}

  void Update(float dt, const SystemMonitor &monitor) override {
    m_currentUsage = static_cast<float>(monitor.GetCpuUsage());
    m_pulse += 0.1f + (GetEffectiveUsage() * 0.5f);
  }

  void Draw(Shader *shader, const Mat4 &sceneTransform) override {
    if (!IsEnabled())
      return;
    if (m_config.cpuMetric.meshType == MeshType::None)
      return;
    if (!shader || !shader->IsValid())
      return;

    float u = GetEffectiveUsage();
    float scale = 1.0f + (u * 0.5f) + (std::sin(m_pulse) * 0.1f);

    Mat4 model = Mat4Multiply(sceneTransform, Mat4Scale(scale, scale, scale));
    shader->Use();
    shader->SetMat4("uModel", model.m.data());
    shader->SetVec3("uColor", u, 0.2f, 1.0f - u);
    DrawMesh(GetMesh());
  }

  void Cleanup() override {}

  bool IsEnabled() const override { return m_config.cpuMetric.enabled; }

private:
  float GetEffectiveUsage() const {
    float effective = m_currentUsage - m_config.cpuMetric.threshold;
    if (effective < 0.0f)
      effective = 0.0f;
    float u = (effective / (100.0f - m_config.cpuMetric.threshold)) *
              m_config.cpuMetric.strength;
    return (u > 1.0f) ? 1.0f : u;
  }

  Mesh &GetMesh() {
    switch (m_config.cpuMetric.meshType) {
    case MeshType::Cube:
      return m_cubeMesh;
    case MeshType::Ring:
      return m_ringMesh;
    default:
      return m_sphereMesh;
    }
  }

  const Config &m_config;
  Mesh &m_sphereMesh;
  Mesh &m_cubeMesh;
  Mesh &m_ringMesh;
  float m_currentUsage = 0.0f;
  float m_pulse = 0.0f;
};
