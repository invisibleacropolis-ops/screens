#pragma once

#include "../glad/glad.h"
#include "PostProcessConfig.h"
#include "VisualizerLayer.h"
#include <algorithm>
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);

    for (auto *layer : layers) {
      if (!layer)
        continue;

      const auto &fx = layer->GetFXConfig();

      // Set blend mode based on layer config
      switch (fx.blendMode) {
      case BlendMode::Additive:
      case BlendMode::Multiply:
      case BlendMode::Screen:
        glBlendFunc(GL_ONE, GL_ONE);
        break;
      case BlendMode::Normal:
      default:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      }

      // Draw layer's texture as fullscreen quad
      // (In real implementation, use a quad shader)
      DrawLayerTexture(layer->GetColorTexture(), fx.opacity);
    }

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
  }

private:
  void DrawLayerTexture(GLuint texture, float opacity) {
    // Placeholder - actual implementation would use a fullscreen quad shader
    // that samples the texture with the given opacity
    (void)texture;
    (void)opacity;
  }

  GLuint m_outputFbo = 0;
  GLuint m_outputTex = 0;
  int m_width = 0;
  int m_height = 0;
};
