#pragma once

#include "../graphics/Shader.h"
#include "IVisualizer.h"
#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>

// CPU Visualization: surreal sci-fi mesh dreamscape.
class CPUVisualizer : public IVisualizer {
public:
  explicit CPUVisualizer(const Config &config) : m_config(config) {
    m_gridX = m_config.cpuGridSize;
    m_gridZ = m_config.cpuGridSize;
    m_history.resize(m_gridZ, 0.0f);
    m_burstHistory.resize(m_gridZ, 0.0f);
  }

  void Init() override {
    m_indices.clear();
    for (int z = 0; z < m_gridZ - 1; ++z) {
      for (int x = 0; x < m_gridX - 1; ++x) {
        const unsigned int i0 = static_cast<unsigned int>(z * m_gridX + x);
        const unsigned int i1 = static_cast<unsigned int>(z * m_gridX + x + 1);
        const unsigned int i2 =
            static_cast<unsigned int>((z + 1) * m_gridX + x);
        const unsigned int i3 =
            static_cast<unsigned int>((z + 1) * m_gridX + x + 1);

        m_indices.push_back(i0);
        m_indices.push_back(i2);
        m_indices.push_back(i1);

        m_indices.push_back(i1);
        m_indices.push_back(i2);
        m_indices.push_back(i3);
      }
    }

    if (m_vao == 0)
      glGenVertexArrays(1, &m_vao);
    if (m_vbo == 0)
      glGenBuffers(1, &m_vbo);
    if (m_ibo == 0)
      glGenBuffers(1, &m_ibo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    const size_t vertexSize = (3 + 3 + 2) * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, m_gridX * m_gridZ * vertexSize, nullptr,
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void *>(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void *>(6 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_indices.size() * sizeof(unsigned int), m_indices.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
  }

  void Update(float dt, const SystemMonitor &monitor) override {
    if (m_config.cpuGridSize != m_gridX) {
      m_gridX = m_config.cpuGridSize;
      m_gridZ = m_config.cpuGridSize;
      m_history.assign(m_gridZ, 0.0f);
      m_burstHistory.assign(m_gridZ, 0.0f);
      Init();
    }

    m_time += dt;
    m_updateTimer += dt;

    const float usage =
        std::clamp(static_cast<float>(monitor.GetCpuUsage()) / 100.0f, 0.0f,
                   1.0f);

    const float delta = usage - m_prevUsage;
    m_prevUsage = usage;

    const float spike = std::clamp((std::fabs(delta) - 0.01f) * 16.0f, 0.0f,
                                   1.0f);
    if (spike > m_burstEnergy) {
      m_burstEnergy += (spike - m_burstEnergy) * 0.55f;
    } else {
      m_burstEnergy *= std::exp(-dt * 3.2f);
    }

    m_macroPhase += dt * (0.20f + usage * 0.55f);
    m_mesoPhase += dt * (0.35f + m_burstEnergy * 1.6f);

    if (m_updateTimer > 0.033f) {
      m_updateTimer = 0.0f;

      m_spectrum.resize(10);
      for (int i = 0; i < 10; ++i) {
        m_spectrum[i] = monitor.GetSpectrumBand(i);
      }

      if (m_history.size() >= static_cast<size_t>(m_gridZ)) {
        m_history.pop_back();
      }
      if (m_burstHistory.size() >= static_cast<size_t>(m_gridZ)) {
        m_burstHistory.pop_back();
      }

      m_history.push_front(usage);
      m_burstHistory.push_front(m_burstEnergy);
    }
  }

  void Draw(Shader *shader, const Mat4 &sceneTransform) override {
    if (!IsEnabled() || !shader)
      return;

    UpdateMesh();

    shader->Use();

    Mat4 transform = Mat4Translate(-2.0f, m_config.cpuYOffset, 0.0f);
    transform = Mat4Multiply(sceneTransform, transform);
    Mat4 scale = Mat4Scale(4.2f, 2.2f, 4.2f);
    Mat4 model = Mat4Multiply(transform, scale);

    shader->SetMat4("uModel", model.m.data());
    shader->SetFloat("uTime", m_time);
    shader->SetFloat("uEnergy", m_currentUsageSmoothed);
    shader->SetFloat("uBurst", m_burstEnergy);
    shader->SetFloat("uPalettePhase", m_macroPhase * 0.35f + m_mesoPhase * 0.2f);

    const float t = std::clamp(0.2f + 0.65f * m_currentUsageSmoothed +
                                   0.15f * m_burstEnergy,
                               0.0f, 1.0f);
    float r = 0.20f + 0.50f * std::sin(6.28318f * (t + 0.08f));
    float g = 0.20f + 0.50f * std::sin(6.28318f * (t + 0.40f));
    float b = 0.25f + 0.55f * std::sin(6.28318f * (t + 0.72f));
    r = std::clamp(r, 0.0f, 1.0f);
    g = std::clamp(g, 0.0f, 1.0f);
    b = std::clamp(b, 0.0f, 1.0f);
    shader->SetVec3("uColor", r, g, b);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }

  void Cleanup() override {
    if (m_vao) {
      glDeleteVertexArrays(1, &m_vao);
      m_vao = 0;
    }
    if (m_vbo) {
      glDeleteBuffers(1, &m_vbo);
      m_vbo = 0;
    }
    if (m_ibo) {
      glDeleteBuffers(1, &m_ibo);
      m_ibo = 0;
    }
  }

  bool IsEnabled() const override { return m_config.cpuMetric.enabled; }

private:
  float InterpolateSpectrum(float x01) const {
    if (m_spectrum.empty())
      return 0.0f;
    const float bandPos = x01 * 9.0f;
    int idx = static_cast<int>(bandPos);
    idx = std::clamp(idx, 0, 9);
    const float frac = bandPos - static_cast<float>(idx);

    const float a = (idx < static_cast<int>(m_spectrum.size())) ? m_spectrum[idx]
                                                                 : 0.0f;
    const float b = (idx + 1 < static_cast<int>(m_spectrum.size()))
                        ? m_spectrum[idx + 1]
                        : a;
    return a * (1.0f - frac) + b * frac;
  }

  void UpdateMesh() {
    std::vector<float> heights(static_cast<size_t>(m_gridX * m_gridZ), 0.0f);
    std::vector<float> vertices;
    vertices.resize(static_cast<size_t>(m_gridX * m_gridZ * 8), 0.0f);

    m_currentUsageSmoothed = m_currentUsageSmoothed * 0.92f +
                             (m_history.empty() ? 0.0f : m_history.front()) * 0.08f;

    const float widthStep = 1.0f / static_cast<float>(m_gridX - 1);
    const float depthStep = 1.0f / static_cast<float>(m_gridZ - 1);

    for (int z = 0; z < m_gridZ; ++z) {
      const float zPos = static_cast<float>(z) * depthStep;
      const float zCentered = (zPos - 0.5f) * 2.0f;

      const float usageHist =
          (z < static_cast<int>(m_history.size())) ? m_history[z] : 0.0f;
      const float burstHist =
          (z < static_cast<int>(m_burstHistory.size())) ? m_burstHistory[z] : 0.0f;

      for (int x = 0; x < m_gridX; ++x) {
        const float xPos = static_cast<float>(x) * widthStep;
        const float xCentered = (xPos - 0.5f) * 2.0f;

        const float spectrumH = InterpolateSpectrum(xPos);

        const float macroWave =
            std::sin(xCentered * 2.7f + zCentered * 1.9f + m_macroPhase) * 0.20f +
            std::cos(zCentered * 2.1f - m_macroPhase * 0.8f) * 0.16f;

        const float mesoWave =
            std::sin(xCentered * 9.0f + m_mesoPhase + spectrumH * 5.5f) *
            (0.06f + 0.18f * spectrumH);

        const float fold = std::abs(std::sin(xCentered * 4.2f + zCentered * 3.0f +
                                             m_time * 0.85f));

        const float silhouette = std::pow(std::clamp(usageHist, 0.0f, 1.0f), 1.25f) *
                                 1.55f;

        float yPos = silhouette;
        yPos += macroWave * (0.45f + usageHist * 0.65f);
        yPos += mesoWave;
        yPos += fold * (0.04f + burstHist * 0.22f);

        const float depthMask = std::clamp(1.0f - zPos * 0.82f, 0.12f, 1.0f);
        const float dreamBreath =
            0.92f + 0.18f * std::sin(m_time * 0.35f + zPos * 5.2f + xPos * 1.8f);
        yPos *= depthMask * dreamBreath;

        heights[static_cast<size_t>(z * m_gridX + x)] = yPos;
      }
    }

    for (int z = 0; z < m_gridZ; ++z) {
      for (int x = 0; x < m_gridX; ++x) {
        const int xm = (x > 0) ? x - 1 : x;
        const int xp = (x + 1 < m_gridX) ? x + 1 : x;
        const int zm = (z > 0) ? z - 1 : z;
        const int zp = (z + 1 < m_gridZ) ? z + 1 : z;

        const float hL = heights[static_cast<size_t>(z * m_gridX + xm)];
        const float hR = heights[static_cast<size_t>(z * m_gridX + xp)];
        const float hD = heights[static_cast<size_t>(zm * m_gridX + x)];
        const float hU = heights[static_cast<size_t>(zp * m_gridX + x)];

        const Vec3 n = Vec3Normalize(Vec3{-hR + hL, 2.0f, -hU + hD});

        const float xPos = static_cast<float>(x) / static_cast<float>(m_gridX - 1);
        const float zPos = static_cast<float>(z) / static_cast<float>(m_gridZ - 1);
        const float yPos = heights[static_cast<size_t>(z * m_gridX + x)];

        const size_t base = static_cast<size_t>((z * m_gridX + x) * 8);
        vertices[base + 0] = xPos;
        vertices[base + 1] = yPos;
        vertices[base + 2] = zPos;

        vertices[base + 3] = n.x;
        vertices[base + 4] = n.y;
        vertices[base + 5] = n.z;

        vertices[base + 6] = xPos;
        vertices[base + 7] = zPos;
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float),
                    vertices.data());
  }

  const Config &m_config;

  int m_gridX = 40;
  int m_gridZ = 40;

  std::deque<float> m_history;
  std::deque<float> m_burstHistory;
  std::vector<float> m_spectrum;

  float m_time = 0.0f;
  float m_updateTimer = 0.0f;
  float m_prevUsage = 0.0f;
  float m_burstEnergy = 0.0f;
  float m_currentUsageSmoothed = 0.0f;
  float m_macroPhase = 0.0f;
  float m_mesoPhase = 0.0f;

  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ibo = 0;
  std::vector<unsigned int> m_indices;
};
