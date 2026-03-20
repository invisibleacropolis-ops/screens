#pragma once

#include "../fractal/FractalSignalProcessor.h"
#include "IVisualizer.h"
#include <vector>

class FractalSurfaceVisualizer : public IVisualizer {
public:
  explicit FractalSurfaceVisualizer(const Config &config);

  void Init() override;
  void Update(float dt, const SystemMonitor &monitor) override;
  void Draw(Shader *shader, const Mat4 &sceneTransform) override;
  void Cleanup() override;
  bool IsEnabled() const override;

private:
  struct Vertex {
    float px;
    float py;
    float pz;
    float nx;
    float ny;
    float nz;
  };

  void BuildGrid();
  void UpdateMesh();
  float EvalHeight(float x, float z) const;
  float Noise2D(float x, float y) const;
  float SmoothNoise(float x, float y) const;
  float FBm(float x, float y, int octaves, float lacunarity, float gain) const;

  const Config &m_config;
  FractalSignalProcessor m_signalProcessor;

  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ibo = 0;

  std::vector<Vertex> m_vertices;
  std::vector<unsigned int> m_indices;

  int m_resX = 96;
  int m_resZ = 96;
  float m_time = 0.0f;
  float m_gridScale = 6.0f;
};
