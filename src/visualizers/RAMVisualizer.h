#pragma once

#include "IVisualizer.h"
#include <cmath>

// RAM Visualization: A grid of shapes that fills up based on memory usage
class RAMVisualizer : public IVisualizer {
public:
  RAMVisualizer(const Config &config, Mesh &sphereMesh, Mesh &cubeMesh,
                Mesh &ringMesh)
      : m_config(config), m_sphereMesh(sphereMesh), m_cubeMesh(cubeMesh),
        m_ringMesh(ringMesh) {}

  void Init() override {}

  void Update(float dt, const SystemMonitor &monitor) override {
    float targetUsage = static_cast<float>(monitor.GetRamUsage());
    m_currentUsage += (targetUsage - m_currentUsage) * dt * 2.0f; // Soft Lerp
    m_pulse += dt * (1.0f + GetEffectiveUsage() * 0.5f);
  }

  void Draw(Shader *shader, const Mat4 &sceneTransform) override {
    if (!IsEnabled() || !shader)
      return;

    float u = GetEffectiveUsage();
    // Pulse scale: Base + Usage + SineWave
    float scale = 1.0f + (u * 1.5f) + (std::sin(m_pulse) * 0.1f);

    Mat4 transform = Mat4Translate(0.0f, 0.0f, 0.0f);
    transform = Mat4Multiply(sceneTransform, transform);

    Mat4 model = Mat4Multiply(transform, Mat4Scale(scale, scale, scale));

    // Rotate slowly
    model = Mat4Multiply(model, Mat4RotateY(m_pulse * 0.5f));
    model = Mat4Multiply(model, Mat4RotateX(m_pulse * 0.3f));

    shader->Use();
    shader->SetMat4("uModel", model.m.data());

    // Color: Blue/Cyan based on usage
    shader->SetVec3("uColor", 0.1f, 0.5f + (u * 0.5f), 1.0f);

    DrawMesh(GetMesh());
  }

  void Cleanup() override {}

  bool IsEnabled() const override { return m_config.ramMetric.enabled; }

private:
  float GetEffectiveUsage() const {
    // Normalize 0-100 to 0-1 based on Threshold/Strength
    float effective = m_currentUsage - m_config.ramMetric.threshold;
    if (effective < 0)
      effective = 0;
    float range = 100.0f - m_config.ramMetric.threshold;
    if (range <= 0.001f)
      range = 100.0f;

    float u = (effective / range) * m_config.ramMetric.strength;
    return (u > 1.0f) ? 1.0f : u;
  }

  Mesh &GetMesh() {
    switch (m_config.ramMetric.meshType) {
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
