#pragma once

#include "../Config.h"
#include "../SystemMonitor.h"
#include "../graphics/LayerCompositor.h"
#include "../graphics/Mesh.h"
#include "../graphics/Shader.h"
#include "../graphics/VisualizerLayer.h"
#include <array>
#include <memory>
#include <windows.h>

// Forward declarations
class Particles;

// Layer indices for easy access
enum class LayerIndex { CPU = 0, RAM = 1, Disk = 2, Network = 3, Count = 4 };

// Engine facade - orchestrates all rendering subsystems
class Engine {
public:
  Engine();
  ~Engine();

  // Lifecycle
  bool Initialize(HWND hwnd);
  void Cleanup();

  // Configuration
  void SetConfig(const Config &config);
  const Config &GetConfig() const { return m_config; }

  // Frame rendering
  void Render(int width, int height);

  // Access to subsystems
  SystemMonitor *GetSystemMonitor() { return m_systemMonitor.get(); }
  Shader *GetMainShader() { return m_mainShader.get(); }

  // Layer access
  VisualizerLayer &GetLayer(LayerIndex idx) {
    return m_layers[static_cast<int>(idx)];
  }
  const VisualizerLayer &GetLayer(LayerIndex idx) const {
    return m_layers[static_cast<int>(idx)];
  }

  // Update layer FX config from settings
  void SetLayerFXConfig(LayerIndex idx, const PostProcessConfig &config);

  // Get all layers for compositor
  std::array<VisualizerLayer *, 4> GetLayerPointers();
  void UpdateLayersFromConfig();

private:
  // OpenGL context setup
  bool InitializeOpenGLContext(HWND hwnd);
  void SetupShaders();
  void SetupMeshes();
  void SetupLayers(int width, int height);

  // Per-frame operations
  void UpdateMetrics(float dt);
  void UpdateScene(float dt);
  void RenderToLayers(int width, int height);
  void CompositeLayers(int width, int height);

  // Configuration
  Config m_config;
  HWND m_hwnd = nullptr;
  HDC m_hdc = nullptr;
  HGLRC m_hrc = nullptr;

  // Subsystems
  std::unique_ptr<SystemMonitor> m_systemMonitor;
  std::unique_ptr<Particles> m_particles;
  std::unique_ptr<Shader> m_mainShader;
  std::unique_ptr<Shader> m_skyboxShader;
  std::unique_ptr<Shader> m_postProcessShader;

  // Meshes (owned by engine)
  Mesh m_cubeMesh;
  Mesh m_sphereMesh;
  Mesh m_ringMesh;

  // Visualizer Layers - 4 fixed layers (CPU, RAM, Disk, Network)
  std::array<VisualizerLayer, static_cast<int>(LayerIndex::Count)> m_layers;
  LayerCompositor m_compositor;

  // Scene state
  Mat4 m_sceneTransform;
  float m_cameraAngle = 0.0f;

  // Frame timing
  LARGE_INTEGER m_timerFreq;
  LARGE_INTEGER m_lastFrameTime;
  bool m_hasFrameTime = false;

  // Current screen dimensions
  int m_screenWidth = 0;
  int m_screenHeight = 0;
};
