#pragma once

#include "../Config.h"
#include "../SystemMonitor.h"
#include "../engine/Math.h"
#include "../graphics/Mesh.h"
#include "../graphics/Shader.h"


// Base interface for all metric visualizers
class IVisualizer {
public:
  virtual ~IVisualizer() = default;

  // Initialize any resources needed
  virtual void Init() = 0;

  // Update state based on delta time and metrics
  virtual void Update(float dt, const SystemMonitor &monitor) = 0;

  // Draw the visualization using the provided shader and scene transform
  virtual void Draw(Shader *shader, const Mat4 &sceneTransform) = 0;

  // Cleanup resources
  virtual void Cleanup() = 0;

  // Check if this visualizer is enabled
  virtual bool IsEnabled() const = 0;
};
