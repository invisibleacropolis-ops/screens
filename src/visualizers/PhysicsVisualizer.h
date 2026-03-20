#pragma once

#include "../physics/PhysicsWorld.h"
#include "IVisualizer.h"
#include <memory>


class PhysicsVisualizer : public IVisualizer {
public:
  PhysicsVisualizer() { m_physicsWorld = std::make_unique<PhysicsWorld>(); }

  virtual ~PhysicsVisualizer() { Cleanup(); }

  void Init() override {
    m_physicsWorld->Init();
    OnInit();
  }

  void Update(float dt, const SystemMonitor &monitor) override {
    // Step physics
    m_physicsWorld->Update(dt);
    // subclass update
    OnUpdate(dt, monitor);
  }

  void Cleanup() override {
    OnCleanup();
    m_physicsWorld->Cleanup();
  }

protected:
  // Hooks for subclasses
  virtual void OnInit() = 0;
  virtual void OnUpdate(float dt, const SystemMonitor &monitor) = 0;
  virtual void OnCleanup() = 0;

  std::unique_ptr<PhysicsWorld> m_physicsWorld;
};
