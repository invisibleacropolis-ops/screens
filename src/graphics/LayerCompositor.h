#pragma once

#include "../Logger.h"
#include "../glad/glad.h"
#include "PostProcessConfig.h"
#include "Shader.h"
#include "VisualizerLayer.h"
#include <algorithm>
#include <memory>
#include <vector>

// LayerCompositor - blends all visualizer layers into final output
class LayerCompositor {
public:
  LayerCompositor() = default;
  ~LayerCompositor() { Cleanup(); }

  // Initialize with screen dimensions
  bool Initialize(int width, int height) {
    m_width = width;
    m_height = height;

    // Create output FBO for composited result
    glGenFramebuffers(1, &m_outputFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_outputFbo);

    glGenTextures(1, &m_outputTex);
    glBindTexture(GL_TEXTURE_2D, m_outputTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_outputTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      Logger::LogS("Compositor FBO Incomplete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize Shader
    m_shader = std::make_unique<Shader>("assets/shaders/passthrough.vert",
                                        "assets/shaders/passthrough.frag");

    // Initialize Quad VAO
    float quadVertices[] = {// positions   // texCoords
                            -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                            0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                            1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glBindVertexArray(0);

    return true;
  }

  void Resize(int width, int height) {
    Cleanup();
    Initialize(width, height);
  }

  // Composite all layers in render order
  void Composite(std::vector<VisualizerLayer *> &layers) {
    // Sort by render order
    std::sort(layers.begin(), layers.end(),
              [](const VisualizerLayer *a, const VisualizerLayer *b) {
                return a->GetRenderOrder() < b->GetRenderOrder();
              });

    glBindFramebuffer(GL_FRAMEBUFFER, m_outputFbo);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to transparent
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST); // Disable depth for 2D composition

    for (auto *layer : layers) {
      if (!layer)
        continue;

      const auto &fx = layer->GetFXConfig();

      // Set blend mode based on layer config
      switch (fx.blendMode) {
      case BlendMode::Additive:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
      case BlendMode::Multiply:
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        break;
      case BlendMode::Screen:
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        break;
      case BlendMode::Normal:
      default:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      }

      DrawLayerTexture(layer->GetColorTexture(), fx.opacity);
    }

    glDisable(GL_BLEND);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // Draw the final composited texture to the default framebuffer
  void Present() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // 2D pass
    DrawLayerTexture(m_outputTex, 1.0f);
    glEnable(GL_DEPTH_TEST);
  }

  GLuint GetOutputTexture() const { return m_outputTex; }

  void Cleanup() {
    if (m_outputFbo) {
      glDeleteFramebuffers(1, &m_outputFbo);
      m_outputFbo = 0;
    }
    if (m_outputTex) {
      glDeleteTextures(1, &m_outputTex);
      m_outputTex = 0;
    }
    if (m_quadVAO) {
      glDeleteVertexArrays(1, &m_quadVAO);
      m_quadVAO = 0;
    }
    if (m_quadVBO) {
      glDeleteBuffers(1, &m_quadVBO);
      m_quadVBO = 0;
    }
    m_shader.reset();
  }

private:
  void DrawLayerTexture(GLuint texture, float opacity) {
    if (m_shader) {
      m_shader->Use();
      m_shader->SetInt("screenTexture", 0);
      m_shader->SetFloat("opacity", opacity);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      glBindVertexArray(m_quadVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindVertexArray(0);
    }
  }

  GLuint m_outputFbo = 0;
  GLuint m_outputTex = 0;
  int m_width = 0;
  int m_height = 0;

  std::unique_ptr<Shader> m_shader;
  GLuint m_quadVAO = 0;
  GLuint m_quadVBO = 0;
};
