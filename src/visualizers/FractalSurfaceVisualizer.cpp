#include "FractalSurfaceVisualizer.h"

#include "../glad/glad.h"
#include <algorithm>
#include <cmath>

FractalSurfaceVisualizer::FractalSurfaceVisualizer(const Config &config)
    : m_config(config) {}

void FractalSurfaceVisualizer::Init() {
  if (m_config.quality == QualityTier::Low) {
    m_resX = 64;
    m_resZ = 64;
  } else if (m_config.quality == QualityTier::Medium) {
    m_resX = 96;
    m_resZ = 96;
  } else {
    m_resX = 128;
    m_resZ = 128;
  }

  BuildGrid();

  if (m_vao == 0)
    glGenVertexArrays(1, &m_vao);
  if (m_vbo == 0)
    glGenBuffers(1, &m_vbo);
  if (m_ibo == 0)
    glGenBuffers(1, &m_ibo);

  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
               m_vertices.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               m_indices.size() * sizeof(unsigned int), m_indices.data(),
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(0));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(3 * sizeof(float)));

  glBindVertexArray(0);
}

void FractalSurfaceVisualizer::Update(float dt, const SystemMonitor &monitor) {
  m_time += dt;
  m_signalProcessor.Update(dt, monitor, m_config);
  UpdateMesh();
}

void FractalSurfaceVisualizer::Draw(Shader *shader, const Mat4 &sceneTransform) {
  if (!IsEnabled() || !shader || !shader->IsValid())
    return;

  const FractalParams &params = m_signalProcessor.GetParams();

  Mat4 translate = Mat4Translate(2.0f, -1.0f, 0.0f);
  Mat4 rotate = Mat4RotateX(-0.65f);
  float scaleY = 1.0f + params.energy * 0.5f;
  Mat4 scale = Mat4Scale(1.0f, scaleY, 1.0f);
  Mat4 model = Mat4Multiply(sceneTransform, Mat4Multiply(translate, Mat4Multiply(rotate, scale)));

  shader->Use();
  shader->SetMat4("uModel", model.m.data());
  shader->SetFloat("uTime", m_time);
  shader->SetFloat("uEnergy", params.energy);
  shader->SetFloat("uPalettePhase", params.palettePhase);

  float hue = std::fmod(params.palettePhase * 0.09f, 1.0f);
  float r = 0.25f + 0.75f * std::sin((hue + 0.0f) * 6.28318f) * 0.5f + 0.25f;
  float g = 0.25f + 0.75f * std::sin((hue + 0.33f) * 6.28318f) * 0.5f + 0.25f;
  float b = 0.25f + 0.75f * std::sin((hue + 0.66f) * 6.28318f) * 0.5f + 0.25f;
  shader->SetVec3("uColor", r, g, b);

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                 GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

void FractalSurfaceVisualizer::Cleanup() {
  if (m_ibo) {
    glDeleteBuffers(1, &m_ibo);
    m_ibo = 0;
  }
  if (m_vbo) {
    glDeleteBuffers(1, &m_vbo);
    m_vbo = 0;
  }
  if (m_vao) {
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
  }
}

bool FractalSurfaceVisualizer::IsEnabled() const {
  return m_config.fractalEnabled && m_config.networkMetric.enabled;
}

void FractalSurfaceVisualizer::BuildGrid() {
  m_vertices.clear();
  m_indices.clear();

  m_vertices.resize(static_cast<size_t>(m_resX * m_resZ));

  for (int z = 0; z < m_resZ; ++z) {
    for (int x = 0; x < m_resX; ++x) {
      const float u = static_cast<float>(x) / static_cast<float>(m_resX - 1);
      const float v = static_cast<float>(z) / static_cast<float>(m_resZ - 1);

      Vertex &vert = m_vertices[static_cast<size_t>(z * m_resX + x)];
      vert.px = (u - 0.5f) * m_gridScale;
      vert.py = 0.0f;
      vert.pz = (v - 0.5f) * m_gridScale;
      vert.nx = 0.0f;
      vert.ny = 1.0f;
      vert.nz = 0.0f;
    }
  }

  for (int z = 0; z < m_resZ - 1; ++z) {
    for (int x = 0; x < m_resX - 1; ++x) {
      unsigned int i0 = static_cast<unsigned int>(z * m_resX + x);
      unsigned int i1 = static_cast<unsigned int>(z * m_resX + x + 1);
      unsigned int i2 = static_cast<unsigned int>((z + 1) * m_resX + x);
      unsigned int i3 = static_cast<unsigned int>((z + 1) * m_resX + x + 1);

      m_indices.push_back(i0);
      m_indices.push_back(i2);
      m_indices.push_back(i1);

      m_indices.push_back(i1);
      m_indices.push_back(i2);
      m_indices.push_back(i3);
    }
  }
}

void FractalSurfaceVisualizer::UpdateMesh() {
  const FractalParams &params = m_signalProcessor.GetParams();

  for (int z = 0; z < m_resZ; ++z) {
    for (int x = 0; x < m_resX; ++x) {
      Vertex &vert = m_vertices[static_cast<size_t>(z * m_resX + x)];
      vert.py = EvalHeight(vert.px, vert.pz);
    }
  }

  for (int z = 0; z < m_resZ; ++z) {
    for (int x = 0; x < m_resX; ++x) {
      const int xm = (x > 0) ? x - 1 : x;
      const int xp = (x + 1 < m_resX) ? x + 1 : x;
      const int zm = (z > 0) ? z - 1 : z;
      const int zp = (z + 1 < m_resZ) ? z + 1 : z;

      const float hL = m_vertices[static_cast<size_t>(z * m_resX + xm)].py;
      const float hR = m_vertices[static_cast<size_t>(z * m_resX + xp)].py;
      const float hD = m_vertices[static_cast<size_t>(zm * m_resX + x)].py;
      const float hU = m_vertices[static_cast<size_t>(zp * m_resX + x)].py;

      Vec3 n = Vec3Normalize(Vec3{-hR + hL, 2.0f, -hU + hD});
      Vertex &vert = m_vertices[static_cast<size_t>(z * m_resX + x)];
      vert.nx = n.x;
      vert.ny = n.y;
      vert.nz = n.z;
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(Vertex),
                  m_vertices.data());

  (void)params;
}

float FractalSurfaceVisualizer::EvalHeight(float x, float z) const {
  const FractalParams &p = m_signalProcessor.GetParams();
  const float t = m_time * p.warpSpeed;

  const float fx = x * p.baseScale;
  const float fz = z * p.baseScale;

  const float warpX = FBm(fx + t * 0.13f, fz - t * 0.19f, p.octaves,
                          p.lacunarity, p.gain);
  const float warpZ = FBm(fx - t * 0.17f, fz + t * 0.11f, p.octaves,
                          p.lacunarity, p.gain);

  const float qx = fx + (warpX - 0.5f) * p.warpAmount * 4.0f;
  const float qz = fz + (warpZ - 0.5f) * p.warpAmount * 4.0f;

  const float base = FBm(qx, qz, p.octaves, p.lacunarity, p.gain) - 0.5f;
  const float ridge = 1.0f - std::fabs(base * 2.0f);
  const float mixed = base * (1.0f - p.ridgeMix) + ridge * p.ridgeMix * 0.5f;

  return mixed * p.amplitude;
}

float FractalSurfaceVisualizer::Noise2D(float x, float y) const {
  const float v = std::sin(x * 127.1f + y * 311.7f + m_config.fractalSeed * 0.01f) *
                  43758.5453123f;
  return v - std::floor(v);
}

float FractalSurfaceVisualizer::SmoothNoise(float x, float y) const {
  const float ix = std::floor(x);
  const float iy = std::floor(y);
  const float fx = x - ix;
  const float fy = y - iy;

  const float a = Noise2D(ix, iy);
  const float b = Noise2D(ix + 1.0f, iy);
  const float c = Noise2D(ix, iy + 1.0f);
  const float d = Noise2D(ix + 1.0f, iy + 1.0f);

  const float ux = fx * fx * (3.0f - 2.0f * fx);
  const float uy = fy * fy * (3.0f - 2.0f * fy);

  const float ab = a + (b - a) * ux;
  const float cd = c + (d - c) * ux;
  return ab + (cd - ab) * uy;
}

float FractalSurfaceVisualizer::FBm(float x, float y, int octaves,
                                    float lacunarity, float gain) const {
  float sum = 0.0f;
  float amp = 0.5f;
  float freqX = x;
  float freqY = y;

  for (int i = 0; i < octaves; ++i) {
    sum += SmoothNoise(freqX, freqY) * amp;
    freqX *= lacunarity;
    freqY *= lacunarity;
    amp *= gain;
  }
  return sum;
}
