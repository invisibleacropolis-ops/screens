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
    m_currentUsage = static_cast<float>(monitor.GetRamUsage());
  }

  void Draw(Shader *shader, const Mat4 &sceneTransform) override {
    if (!IsEnabled())
      return;
    if (m_config.ramMetric.meshType == MeshType::None)
      return;
    if (!shader || !shader->IsValid())
      return;

    float u = GetEffectiveUsage();
    int litCubes = static_cast<int>(u * 100);

    float spacing = 0.25f;
    float startX = -(10 * spacing) / 2.0f;
    float startZ = -(10 * spacing) / 2.0f;

    for (int z = 0; z < 10; z++) {
      for (int x = 0; x < 10; x++) {
        int idx = z * 10 + x;
        float tx = startX + x * spacing;
        float tz = startZ + z * spacing;
        Mat4 translate = Mat4Translate(tx, -2.5f, tz);
        float size = (idx < litCubes) ? 0.2f : 0.1f;
        Mat4 scale = Mat4Scale(size, size, size);
        Mat4 model =
            Mat4Multiply(sceneTransform, Mat4Multiply(translate, scale));

        shader->Use();
        shader->SetMat4("uModel", model.m.data());
        if (idx < litCubes) {
          shader->SetVec3("uColor", u > 0.8f ? 1.0f : 0.0f, 1.0f - u, 0.0f);
        } else {
          shader->SetVec3("uColor", 0.1f, 0.1f, 0.1f);
        }
        DrawMesh(GetMesh());
      }
    }
  }

  void Cleanup() override {}

  bool IsEnabled() const override { return m_config.ramMetric.enabled; }

private:
  float GetEffectiveUsage() const {
    float effective = m_currentUsage - m_config.ramMetric.threshold;
    if (effective < 0.0f)
      effective = 0.0f;
    float u = (effective / (100.0f - m_config.ramMetric.threshold)) *
              m_config.ramMetric.strength;
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
};
