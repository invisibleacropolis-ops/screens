#pragma once

#include "../glad/glad.h"
#include "../visualizers/IVisualizer.h"
#include "PostProcessConfig.h"
#include <memory>

// VisualizerLayer - encapsulates a visualizer with its own render target and FX
// Each metric (CPU, RAM, Disk, Network) gets its own layer
class VisualizerLayer {
public:
  VisualizerLayer() = default;
  ~VisualizerLayer() { Cleanup(); }

  // Disable copying (owns OpenGL resources)
  VisualizerLayer(const VisualizerLayer &) = delete;
  VisualizerLayer &operator=(const VisualizerLayer &) = delete;

  // Enable moving
  VisualizerLayer(VisualizerLayer &&other) noexcept {
    *this = std::move(other);
  }

  VisualizerLayer &operator=(VisualizerLayer &&other) noexcept {
    if (this != &other) {
      Cleanup();
      m_fbo = other.m_fbo;
      m_colorTex = other.m_colorTex;
      m_depthTex = other.m_depthTex;
      m_trailFbo = other.m_trailFbo;
      m_trailTex = other.m_trailTex;
      m_width = other.m_width;
      m_height = other.m_height;
      m_visualizer = std::move(other.m_visualizer);
      m_fxConfig = other.m_fxConfig;
      m_name = other.m_name;

      other.m_fbo = 0;
      other.m_colorTex = 0;
      other.m_depthTex = 0;
      other.m_trailFbo = 0;
      other.m_trailTex = 0;
    }
    return *this;
  }

  // Initialize layer with dimensions
  bool Initialize(int width, int height, const char *name) {
    m_width = width;
    m_height = height;
    m_name = name;
    return CreateFramebuffer(width, height);
  }

  // Resize framebuffers when window changes
  void Resize(int width, int height) {
    if (width != m_width || height != m_height) {
      Cleanup();
      CreateFramebuffer(width, height);
      m_width = width;
      m_height = height;
    }
  }

  // Set the visualizer for this layer
  void SetVisualizer(std::unique_ptr<IVisualizer> viz) {
    m_visualizer = std::move(viz);
  }

  IVisualizer *GetVisualizer() { return m_visualizer.get(); }
  const IVisualizer *GetVisualizer() const { return m_visualizer.get(); }

  // FX Configuration
  PostProcessConfig &GetFXConfig() { return m_fxConfig; }
  const PostProcessConfig &GetFXConfig() const { return m_fxConfig; }
  void SetFXConfig(const PostProcessConfig &cfg) { m_fxConfig = cfg; }

  // Render operations
  void BindForRendering() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  // Get output texture for compositing
  GLuint GetColorTexture() const { return m_colorTex; }
  GLuint GetTrailTexture() const { return m_trailTex; }
  GLuint GetFBO() const { return m_fbo; }

  // Layer properties
  const char *GetName() const { return m_name; }
  int GetWidth() const { return m_width; }
  int GetHeight() const { return m_height; }
  int GetRenderOrder() const { return m_fxConfig.renderOrder; }

  // Cleanup resources
  void Cleanup() {
    if (m_fbo) {
      glDeleteFramebuffers(1, &m_fbo);
      m_fbo = 0;
    }
    if (m_colorTex) {
      glDeleteTextures(1, &m_colorTex);
      m_colorTex = 0;
    }
    if (m_depthTex) {
      glDeleteTextures(1, &m_depthTex);
      m_depthTex = 0;
    }
    if (m_trailFbo) {
      glDeleteFramebuffers(1, &m_trailFbo);
      m_trailFbo = 0;
    }
    if (m_trailTex) {
      glDeleteTextures(1, &m_trailTex);
      m_trailTex = 0;
    }
  }

private:
  bool CreateFramebuffer(int width, int height) {
    // Main color/depth FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color texture (RGBA16F for HDR)
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_colorTex, 0);

    // Depth texture
    glGenTextures(1, &m_depthTex);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           m_depthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      return false;
    }

    // Trail buffer for persistence effects
    glGenFramebuffers(1, &m_trailFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_trailFbo);

    glGenTextures(1, &m_trailTex);
    glBindTexture(GL_TEXTURE_2D, m_trailTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_trailTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
  }

  // OpenGL resources
  GLuint m_fbo = 0;
  GLuint m_colorTex = 0;
  GLuint m_depthTex = 0;
  GLuint m_trailFbo = 0; // For trail/persistence effects
  GLuint m_trailTex = 0;

  // Dimensions
  int m_width = 0;
  int m_height = 0;

  // Visualizer
  std::unique_ptr<IVisualizer> m_visualizer;

  // FX settings
  PostProcessConfig m_fxConfig;

  // Layer name (for debugging/settings)
  const char *m_name = "";
};
