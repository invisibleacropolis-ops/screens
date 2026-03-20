#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {}

PhysicsWorld::~PhysicsWorld() { Cleanup(); }

void PhysicsWorld::Init() {
  // Collision configuration contains default collision algorithms
  // Use btSoftBodyRigidBodyCollisionConfiguration for soft body support
  m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

  // Use the default collision dispatcher
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

  // Broadphase: sweep and prune is good for general purpose
  m_broadphase = new btDbvtBroadphase();

  // The default constraint solver
  m_solver = new btSequentialImpulseConstraintSolver();

  // dynamic world
  m_dynamicsWorld = new btSoftRigidDynamicsWorld(
      m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

  // Set gravity
  m_dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

  // Initialize SoftBody world info
  m_softBodyWorldInfo.m_dispatcher = m_dispatcher;
  m_softBodyWorldInfo.m_broadphase = m_broadphase;
  m_softBodyWorldInfo.m_gravity = m_dynamicsWorld->getGravity();
  m_softBodyWorldInfo.m_sparsesdf.Initialize();
}

void PhysicsWorld::Update(float dt) {
  if (m_dynamicsWorld) {
    // Step simulation: timeStep, maxSubSteps, fixedTimeStep
    m_dynamicsWorld->stepSimulation(dt, 5, 1.0f / 60.0f);
  }
}

void PhysicsWorld::Cleanup() {
  // Clean up in reverse order
  if (m_dynamicsWorld) {
    // Remove objects usually handled by caller or we iterate and remove
    // For now, just delete world
    delete m_dynamicsWorld;
    m_dynamicsWorld = nullptr;
  }

  if (m_solver) {
    delete m_solver;
    m_solver = nullptr;
  }

  if (m_broadphase) {
    delete m_broadphase;
    m_broadphase = nullptr;
  }

  if (m_dispatcher) {
    delete m_dispatcher;
    m_dispatcher = nullptr;
  }

  if (m_collisionConfiguration) {
    delete m_collisionConfiguration;
    m_collisionConfiguration = nullptr;
  }
}
