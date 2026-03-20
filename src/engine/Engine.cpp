#include "Engine.h"
#include "../Logger.h"
#include "../Particles.h"
#include "../glad/glad.h"
#include "DebugUtils.h"
#include "Engine.h"

#include "../visualizers/CPUVisualizer.h"
#include "../visualizers/DiskVisualizer.h"
#include "../visualizers/FractalSurfaceVisualizer.h"
#include "../visualizers/RAMVisualizer.h"
#include <cmath>


Engine::Engine() { QueryPerformanceFrequency(&m_timerFreq); }

Engine::~Engine() { Cleanup(); }

bool Engine::Initialize(HWND hwnd) {
  Logger::LogS("Engine::Initialize Start");
  m_hwnd = hwnd;

  if (!InitializeOpenGLContext(hwnd)) {
    Logger::LogS("Failed to initialize OpenGL Context");
    return false;
  }
  Logger::LogS("OpenGL Context Initialized");

  Logger::LogS("Setting up Shaders...");
  SetupShaders();
  Logger::LogS("Setting up Meshes...");
  SetupMeshes();

  // Create system monitor
  m_systemMonitor = std::make_unique<SystemMonitor>();
  m_systemMonitor->Initialize();

  // Initialize layers with default size (will resize on first frame)
  Logger::LogS("Setting up Layers...");
  SetupLayers(1920, 1080);

  // Create visualizers  // Setup Layers
  // CPU Layer
  m_layers[0].Initialize(1920, 1080, "CPU");
  m_layers[0].SetVisualizer(std::make_unique<CPUVisualizer>(m_config));
  m_layers[0].SetFXConfig(m_config.layerConfigs[0]);

  // RAM Layer
  m_layers[static_cast<int>(LayerIndex::RAM)].SetVisualizer(
      std::make_unique<RAMVisualizer>(m_config, m_sphereMesh, m_cubeMesh,
                                      m_ringMesh));

  // Disk Layer
  m_layers[static_cast<int>(LayerIndex::Disk)].SetVisualizer(
      std::make_unique<DiskVisualizer>(m_config, m_sphereMesh, m_cubeMesh,
                                       m_ringMesh));

  // Network layer currently has no dedicated visualizer implementation.
  m_layers[static_cast<int>(LayerIndex::Network)].SetVisualizer(
      std::make_unique<FractalSurfaceVisualizer>(m_config));

  // Apply FX configs from current configuration
  // This will apply defaults/presets if configs are empty, or loaded values if
  // available
  UpdateLayersFromConfig();

  // Initialize all visualizers
  for (auto &layer : m_layers) {
    if (layer.GetVisualizer()) {
      layer.GetVisualizer()->Init();
    }
  }

  m_sceneTransform = Mat4Identity();
  return true;
}

void Engine::Cleanup() {
  // Cleanup layers (includes visualizers)
  for (auto &layer : m_layers) {
    if (layer.GetVisualizer()) {
      layer.GetVisualizer()->Cleanup();
    }
    layer.Cleanup();
  }

  // Cleanup compositor
  m_compositor.Cleanup();

  // Cleanup meshes
  DestroyMesh(m_cubeMesh);
  DestroyMesh(m_sphereMesh);
  DestroyMesh(m_ringMesh);

  // Cleanup shaders
  m_cpuShader.reset();
  m_mainShader.reset();
  m_fractalShader.reset();
  m_skyboxShader.reset();
  m_postProcessShader.reset();

  // Cleanup particles
  m_particles.reset();

  // Cleanup system monitor
  m_systemMonitor.reset();

  // Destroy OpenGL context
  if (m_hrc) {
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(m_hrc);
    m_hrc = nullptr;
  }
  if (m_hdc && m_hwnd) {
    ReleaseDC(m_hwnd, m_hdc);
    m_hdc = nullptr;
  }
}

void Engine::SetConfig(const Config &config) {
  m_config = config;
  UpdateLayersFromConfig();
}

void Engine::UpdateLayersFromConfig() {
  // Apply layer configs from m_config to actual layers
  // with fallback to defaults if config seems empty

  auto ApplyPreset = [&](int index, const PostProcessConfig &preset) {
    if (m_config.layerConfigs[index].transform.width < 0.01f) {
      m_layers[index].SetFXConfig(preset);
      // Update local config to match preset so we don't keep falling back
      m_config.layerConfigs[index] = preset;
    } else {
      m_layers[index].SetFXConfig(m_config.layerConfigs[index]);
    }
  };

  ApplyPreset(static_cast<int>(LayerIndex::CPU), FXPresets::CPUDefault());
  ApplyPreset(static_cast<int>(LayerIndex::RAM), FXPresets::RAMDefault());
  ApplyPreset(static_cast<int>(LayerIndex::Disk), FXPresets::DiskDefault());
  ApplyPreset(static_cast<int>(LayerIndex::Network),
              FXPresets::NetworkDefault());
}

void Engine::SetLayerFXConfig(LayerIndex idx, const PostProcessConfig &config) {
  m_layers[static_cast<int>(idx)].SetFXConfig(config);
  m_config.layerConfigs[static_cast<int>(idx)] = config;
}

std::array<VisualizerLayer *, 4> Engine::GetLayerPointers() {
  return {&m_layers[0], &m_layers[1], &m_layers[2], &m_layers[3]};
}

void Engine::Render(int width, int height) {
  // Handle resize
  if (width != m_screenWidth || height != m_screenHeight) {
    m_screenWidth = width;
    m_screenHeight = height;
    for (auto &layer : m_layers) {
      layer.Resize(width, height);
    }
    m_compositor.Resize(width, height);
  }

  // Calculate delta time
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  float dt = 0.016f; // Default 60fps
  if (m_hasFrameTime && m_timerFreq.QuadPart > 0) {
    dt = static_cast<float>(now.QuadPart - m_lastFrameTime.QuadPart) /
         static_cast<float>(m_timerFreq.QuadPart);
    if (dt > 0.1f)
      dt = 0.1f;
    if (dt < 0.001f)
      dt = 0.001f;
  }
  m_lastFrameTime = now;
  m_hasFrameTime = true;

  // Update systems
  UpdateMetrics(dt);
  UpdateScene(dt);

  // Render to individual layers
  RenderToLayers(width, height);

  // Composite all layers
  CompositeLayers(width, height);

  // Present final result to screen
  m_compositor.Present();

  // Swap buffers
  if (!SwapBuffers(m_hdc)) {
    Logger::LogS("SwapBuffers Failed!");
  }
}

void Engine::UpdateMetrics(float dt) {
  if (m_systemMonitor) {
    m_systemMonitor->Update();
  }

  // Update all visualizers in layers
  for (auto &layer : m_layers) {
    auto *viz = layer.GetVisualizer();
    if (viz && viz->IsEnabled() && m_systemMonitor) {
      viz->Update(dt, *m_systemMonitor);
    }
  }
}

void Engine::UpdateScene(float dt) {
  // Update camera rotation
  m_cameraAngle += m_config.rotationSpeed * dt;
  if (m_cameraAngle > 360.0f)
    m_cameraAngle -= 360.0f;
}

void Engine::RenderToLayers(int width, int height) {
  // Render each visualizer to its own layer
  for (int i = 0; i < static_cast<int>(LayerIndex::Count); ++i) {
    auto &layer = m_layers[i];
    auto *viz = layer.GetVisualizer();
    if (!viz)
      continue;

    const auto &fx = layer.GetFXConfig();
    if (!fx.transform.visible)
      continue;

    // Bind layer's framebuffer
    layer.BindForRendering();

    // Apply layer transform for viewport
    int lx = static_cast<int>(fx.transform.x * width);
    int ly = static_cast<int>(fx.transform.y * height);
    int lw = static_cast<int>(fx.transform.width * width);
    int lh = static_cast<int>(fx.transform.height * height);
    if (lw <= 0 || lh <= 0) {
      layer.Unbind();
      continue;
    }
    glViewport(lx, ly, lw, lh);

    // Draw visualizer
    Shader *shaderToUse = m_mainShader.get();
    if (i == static_cast<int>(LayerIndex::CPU) && m_cpuShader &&
        m_cpuShader->IsValid()) {
      shaderToUse = m_cpuShader.get();
    } else if (i == static_cast<int>(LayerIndex::Network) && m_fractalShader &&
        m_fractalShader->IsValid()) {
      shaderToUse = m_fractalShader.get();
    }

    if (shaderToUse && shaderToUse->IsValid()) {
      shaderToUse->Use();

      // Camera setup

      CheckGLError("Before Camera Setup");

      float aspect = static_cast<float>(lw) / static_cast<float>(lh);
      const float fovDegrees =
          (m_config.fieldOfView > 1.0f) ? m_config.fieldOfView : 45.0f;
      Mat4 projection =
          Mat4Perspective(fovDegrees * (kPi / 180.0f), aspect, 0.1f, 100.0f);

      // Rotate camera around center
      float radius = m_config.cameraDistance;
      float camX = std::sin(m_cameraAngle * (kPi / 180.0f)) * radius;
      float camZ = std::cos(m_cameraAngle * (kPi / 180.0f)) * radius;

      Vec3 eye{camX, m_config.cameraHeight, camZ};
      Vec3 target{0.0f, 0.0f, 0.0f};
      Vec3 up{0.0f, 1.0f, 0.0f};

      Mat4 view = Mat4LookAt(eye, target, up);

      // Log matrices once
      static bool loggedMatrices = false;
      if (!loggedMatrices) {
        LogMatrix("Projection", projection.m.data());
        LogMatrix("View", view.m.data());

         GLint locView = glGetUniformLocation(shaderToUse->GetId(), "uView");
         GLint locProj =
             glGetUniformLocation(shaderToUse->GetId(), "uProjection");
        std::stringstream ss;
        ss << "Uniform locs - uView: " << locView
           << ", uProjection: " << locProj;
        Logger::LogS(ss.str());

        loggedMatrices = true;
      }

       shaderToUse->SetMat4("uProjection", projection.m.data());
       shaderToUse->SetMat4("uView", view.m.data());
       CheckGLError("After Setting Matrices");

       viz->Draw(shaderToUse, m_sceneTransform);
       CheckGLError("After Draw");
     }

    layer.Unbind();
  }
}

void Engine::CompositeLayers(int width, int height) {
  // Get layer pointers sorted by render order
  auto layerPtrs = GetLayerPointers();
  std::vector<VisualizerLayer *> layers(layerPtrs.begin(), layerPtrs.end());

  // Composite into final output
  m_compositor.Composite(layers);
}

bool Engine::InitializeOpenGLContext(HWND hwnd) {
  m_hdc = GetDC(hwnd);
  if (!m_hdc)
    return false;

  PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR),
                               1,
                               PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                                   PFD_DOUBLEBUFFER,
                               PFD_TYPE_RGBA,
                               32,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               24,
                               8,
                               0,
                               PFD_MAIN_PLANE,
                               0,
                               0,
                               0,
                               0};

  int format = ChoosePixelFormat(m_hdc, &pfd);
  if (!SetPixelFormat(m_hdc, format, &pfd))
    return false;

  m_hrc = wglCreateContext(m_hdc);
  if (!m_hrc)
    return false;

  if (!wglMakeCurrent(m_hdc, m_hrc))
    return false;

  if (!gladLoadGL()) {
    Logger::LogS("Failed to load GLAD!");
    return false;
  }

  return true;
}

void Engine::SetupShaders() {
  Logger::LogS("Loading CPU Surreal Shader...");
  m_cpuShader = std::make_unique<Shader>("assets/shaders/cpu_surreal.vert",
                                         "assets/shaders/cpu_surreal.frag");
  if (!m_cpuShader->IsValid()) {
    Logger::LogS("Failed to load CPU Surreal Shader.");
  } else {
    Logger::LogS("CPU Surreal Shader Loaded Successfully.");
  }

  Logger::LogS("Loading Main Shader...");
  m_mainShader = std::make_unique<Shader>("assets/shaders/basic.vert",
                                          "assets/shaders/basic.frag");
  if (!m_mainShader->IsValid()) {
    Logger::LogS("Failed to load Main Shader!");
  } else {
    Logger::LogS("Main Shader Loaded Successfully.");
  }

  Logger::LogS("Loading Fractal Surface Shader...");
  m_fractalShader =
      std::make_unique<Shader>("assets/shaders/fractal_surface.vert",
                               "assets/shaders/fractal_surface.frag");
  if (!m_fractalShader->IsValid()) {
    Logger::LogS("Failed to load Fractal Surface Shader.");
  } else {
    Logger::LogS("Fractal Surface Shader Loaded Successfully.");
  }
}

void Engine::SetupMeshes() {
  m_cubeMesh = CreateCubeMesh();
  m_sphereMesh = CreateSphereMesh(32, 16);
  m_ringMesh = CreateRingMesh(64, 0.8f, 1.0f);
}

void Engine::SetupLayers(int width, int height) {
  const char *layerNames[] = {"CPU", "RAM", "Disk", "Network"};

  for (int i = 0; i < static_cast<int>(LayerIndex::Count); ++i) {
    m_layers[i].Initialize(width, height, layerNames[i]);
  }

  m_compositor.Initialize(width, height);
}
