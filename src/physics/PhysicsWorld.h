#pragma once

#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "btBulletDynamicsCommon.h"


#include <memory>
#include <vector>


class PhysicsWorld {
public:
  PhysicsWorld();
  ~PhysicsWorld();

  void Init();
  void Update(float dt);
  void Cleanup();

  btSoftRigidDynamicsWorld *GetWorld() const { return m_dynamicsWorld; }
  btSoftBodyWorldInfo &GetSoftBodyWorldInfo() { return m_softBodyWorldInfo; }

private:
  // Core Bullet components
  btSoftBodyRigidBodyCollisionConfiguration *m_collisionConfiguration = nullptr;
  btCollisionDispatcher *m_dispatcher = nullptr;
  btBroadphaseInterface *m_broadphase = nullptr;
  btSequentialImpulseConstraintSolver *m_solver = nullptr;
  btSoftRigidDynamicsWorld *m_dynamicsWorld = nullptr;

  btSoftBodyWorldInfo m_softBodyWorldInfo;
};
