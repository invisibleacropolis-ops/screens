#pragma once

#include "IVisualizer.h"
#include <cmath>

// Disk Visualization: A rotating shape based on disk activity
class DiskVisualizer : public IVisualizer {
public:
  DiskVisualizer(const Config &config, Mesh &sphereMesh, Mesh &cubeMesh,
                 Mesh &ringMesh)
      : m_config(config), m_sphereMesh(sphereMesh), m_cubeMesh(cubeMesh),
        m_ringMesh(ringMesh) {}

  void Init() override {}

  void Update(float dt, const SystemMonitor &monitor) override {
    m_currentUsage = static_cast<float>(monitor.GetDiskUsage());
    float u = GetEffectiveUsage();
    m_rotation += 1.0f + (u * 50.0f);
  }

  void Draw(Shader *shader, const Mat4 &sceneTransform) override {
    if (!IsEnabled())
      return;
    if (m_config.diskMetric.meshType == MeshType::None)
      return;
    if (!shader || !shader->IsValid())
      return;

    float u = GetEffectiveUsage();
    float ringScale = (2.2f + (u * 0.5f)) / 2.2f;

    // Rotate 90 degrees on X axis, then by current rotation on Z
    Mat4 rotate = Mat4Multiply(Mat4RotateX(1.5707964f),
                               Mat4RotateZ(m_rotation * 0.0174533f));
    Mat4 scale = Mat4Scale(ringScale, ringScale, ringScale);
    Mat4 model = Mat4Multiply(sceneTransform, Mat4Multiply(rotate, scale));

    shader->Use();
    shader->SetMat4("uModel", model.m.data());
    shader->SetVec3("uColor", 0.5f + u * 0.5f, 0.5f + u * 0.5f, 0.0f);
    DrawMesh(GetMesh());
  }

  void Cleanup() override {}

  bool IsEnabled() const override { return m_config.diskMetric.enabled; }

private:
  float GetEffectiveUsage() const {
    float effective = m_currentUsage - m_config.diskMetric.threshold;
    if (effective < 0.0f)
      effective = 0.0f;
    float u = (effective / (100.0f - m_config.diskMetric.threshold)) *
              m_config.diskMetric.strength;
    return (u > 1.0f) ? 1.0f : u;
  }

  Mesh &GetMesh() {
    switch (m_config.diskMetric.meshType) {
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
  float m_rotation = 0.0f;
};
