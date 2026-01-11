#pragma once

#include "../graphics/Shader.h"
#include "IVisualizer.h"
#include <cmath>
#include <deque>
#include <vector>

// CPU Visualization: Wireframe 3D Perspective Waveform
// Z-axis: History (scrolling)
// X-axis: Frequency/Variation
// Y-axis: Amplitude (CPU Usage)
class CPUVisualizer : public IVisualizer {
public:
  CPUVisualizer(const Config &config) : m_config(config) {
    // Grid resolution
    m_gridX = 40;
    m_gridZ = 40;
    // Initialize history with 0
    m_history.resize(m_gridZ, 0.0f);
  }

  void Init() override {
    // Generate Indices for a Grid (GL_LINES)
    // Horizontal lines
    for (int z = 0; z < m_gridZ; ++z) {
      for (int x = 0; x < m_gridX - 1; ++x) {
        m_indices.push_back(z * m_gridX + x);
        m_indices.push_back(z * m_gridX + (x + 1));
      }
    }
    // Vertical lines (Time axis)
    for (int x = 0; x < m_gridX; ++x) {
      for (int z = 0; z < m_gridZ - 1; ++z) {
        m_indices.push_back(z * m_gridX + x);
        m_indices.push_back((z + 1) * m_gridX + x);
      }
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Buffer size: Vertices * (Pos(3) + Norm(3) + Tex(2))
    // We only need Pos(3) and Norm(3) really, but let's stick to standard
    // layout if possible Or just tightly pack what usage Basic shader needs.
    // Basic.vert: 0:Pos(3), 1:Normal(3), 2:TexCoord(2)
    // We'll allocate generic size.
    size_t vertexSize = (3 + 3 + 2) * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, m_gridX * m_gridZ * vertexSize, nullptr,
                 GL_DYNAMIC_DRAW);

    // Pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    // Tex
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(6 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_indices.size() * sizeof(unsigned int), m_indices.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
  }

  void Update(float dt, const SystemMonitor &monitor) override {
    m_time += dt;
    m_updateTimer += dt;

    // Use Disk Usage to control extra speed (0.0 to 1.0)
    // Map 0-100 to 0-1
    float disk = static_cast<float>(monitor.GetDiskUsage()) / 100.0f;
    m_extraSpeed += (disk - m_extraSpeed) * dt * 2.0f; // Smooth it

    // Update history at a fixed rate (e.g., 20Hz) to keep scrolling smooth but
    // legible
    if (m_updateTimer > 0.05f) {
      m_updateTimer = 0.0f;

      float usage =
          static_cast<float>(monitor.GetCpuUsage()) / 100.0f; // 0.0 to 1.0

      // Remove oldest
      if (m_history.size() >= m_gridZ) {
        m_history.pop_back();
      }
      // Add newest to front
      m_history.push_front(usage);
    }
  }

  void Draw(Shader *shader, const Mat4 &sceneTransform) override {
    if (!IsEnabled() || !shader)
      return;

    UpdateMesh();

    shader->Use();

    // Scale and position the grid
    Mat4 transform = Mat4Translate(-2.0f, -1.0f, -0.0f); // Center somewhat
    transform = Mat4Multiply(sceneTransform, transform);

    // Apply a scaling to make it occupy the view
    Mat4 scale = Mat4Scale(4.0f, 2.0f, 4.0f);
    Mat4 model = Mat4Multiply(transform, scale);

    shader->SetMat4("uModel", model.m.data());

    // Dynamic color based on current CPU intensity
    float currentLoad = m_history.empty() ? 0.0f : m_history.front();
    float r = currentLoad;
    float g = 1.0f - currentLoad;
    float b = 0.2f + (std::sin(m_time) * 0.1f);
    shader->SetVec3("uColor", r, g, b);

    glBindVertexArray(m_vao);
    glDrawElements(GL_LINES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);
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
  void UpdateMesh() {
    std::vector<float> vertices;
    vertices.reserve(m_gridX * m_gridZ * 8); // 8 floats per vertex

    float widthStep = 1.0f / (m_gridX - 1);
    float depthStep = 1.0f / (m_gridZ - 1);

    for (int z = 0; z < m_gridZ; ++z) {
      float zPos = z * depthStep; // 0.0 to 1.0

      float cpuVal = 0.0f;
      if (z < m_history.size())
        cpuVal = m_history[z];

      for (int x = 0; x < m_gridX; ++x) {
        float xPos = x * widthStep; // 0.0 to 1.0

        // Modulate Height
        // Reverted sensitivity to normal range (1.0 - 1.5)
        float yBase = cpuVal * 1.5f;

        // Modulate secondary wave speed with Disk/RAM usage?
        // Let's use m_speedModifier calculated in Update()

        // Always have some "alive" wave motion
        float baseWave = std::sin(xPos * 12.0f + m_time * 2.0f) * 0.05f;

        // CPU modulates a larger, more chaotic wave
        // Speed is now variable based on m_extraSpeed
        float waveSpeed = 5.0f + (m_extraSpeed * 20.0f);
        float cpuWave =
            std::sin(xPos * 20.0f - m_time * waveSpeed) * (cpuVal * 1.0f);

        // Combine
        float yPos = yBase + baseWave + cpuWave; // Additive height

        // Position
        vertices.push_back(xPos); // X
        vertices.push_back(yPos); // Y
        vertices.push_back(zPos); // Z

        // Normal (Up, for simple lighting)
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);

        // TexCoord
        vertices.push_back(xPos);
        vertices.push_back(zPos);
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float),
                    vertices.data());
  }

  const Config &m_config;

  // Grid config
  int m_gridX = 40;
  int m_gridZ = 40;

  // Data
  std::deque<float> m_history;
  float m_time = 0.0f;
  float m_updateTimer = 0.0f;
  float m_extraSpeed = 0.0f; // Derived from Disk/Other

  // GL
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ibo = 0;
  std::vector<unsigned int> m_indices;
};
