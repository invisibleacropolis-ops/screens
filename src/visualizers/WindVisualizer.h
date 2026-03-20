#pragma once

#include "PhysicsVisualizer.h"
#include <vector>

// Simulates a flag/cloth blowing in the wind
// CPU Usage -> Wind Strength
// RAM Usage -> Cloth color/integrity
class WindVisualizer : public PhysicsVisualizer {
public:
  WindVisualizer(const Config &config);
  virtual ~WindVisualizer();

  void Draw(Shader *shader, const Mat4 &sceneTransform) override;
  bool IsEnabled() const override;

protected:
  void OnInit() override;
  void OnUpdate(float dt, const SystemMonitor &monitor) override;
  void OnCleanup() override;

private:
  void CreateCloth(int resX, int resY);
  void UpdateMeshFromSoftBody();

  const Config &m_config;
  btSoftBody *m_softBody = nullptr;

  // Rendering
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ibo = 0;
  int m_indexCount = 0;

  std::vector<float> m_vertices; // Pos + Normal + UV
  std::vector<unsigned int> m_indices;

  float m_time = 0.0f;
};
