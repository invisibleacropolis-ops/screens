#pragma once

#include "../graphics/Mesh.h"
#include "PhysicsVisualizer.h"
#include <vector>


class GravityVisualizer : public PhysicsVisualizer {
public:
  GravityVisualizer(const Config &config, Mesh *cubeMesh, Mesh *sphereMesh);
  virtual ~GravityVisualizer();

  void Draw(Shader *shader, const Mat4 &sceneTransform) override;
  bool IsEnabled() const override;

protected:
  void OnInit() override;
  void OnUpdate(float dt, const SystemMonitor &monitor) override;
  void OnCleanup() override;

private:
  void SpawnObject();
  void CreateGround();

  const Config &m_config;
  Mesh *m_cubeMesh;
  Mesh *m_sphereMesh;

  // Track created bodies to get their transforms
  struct PhysicsObject {
    btRigidBody *body;
    int type; // 0 = cube, 1 = sphere
    float color[3];
  };
  std::vector<PhysicsObject> m_objects;

  float m_spawnAccumulator = 0.0f;
};
