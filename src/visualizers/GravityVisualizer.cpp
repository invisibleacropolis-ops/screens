#include "GravityVisualizer.h"
#include "../engine/Math.h"
#include <cstdlib>

GravityVisualizer::GravityVisualizer(const Config &config, Mesh *cubeMesh,
                                     Mesh *sphereMesh)
    : m_config(config), m_cubeMesh(cubeMesh), m_sphereMesh(sphereMesh) {}

GravityVisualizer::~GravityVisualizer() { Cleanup(); }

void GravityVisualizer::OnInit() { CreateGround(); }

void GravityVisualizer::CreateGround() {
  // Static ground plane
  btCollisionShape *groundShape = new btBoxShape(btVector3(50, 1, 50));

  btTransform groundTransform;
  groundTransform.setIdentity();
  groundTransform.setOrigin(btVector3(0, -5, 0));

  btScalar mass(0.);
  btVector3 localInertia(0, 0, 0);

  btDefaultMotionState *myMotionState =
      new btDefaultMotionState(groundTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState,
                                                  groundShape, localInertia);
  btRigidBody *body = new btRigidBody(rbInfo);

  m_physicsWorld->GetWorld()->addRigidBody(body);

  // We don't need to track ground in m_objects for rendering if we don't want
  // to draw it, or we can draw a grid there.
}

void GravityVisualizer::OnUpdate(float dt, const SystemMonitor &monitor) {
  if (!m_physicsWorld || !m_physicsWorld->GetWorld())
    return;

  // Disk Usage -> Spawn Rate
  // Range 0-100
  int disk = monitor.GetDiskUsage();
  float spawnRate = 0.2f + (disk * 0.05f); // Base rate + disk boost

  m_spawnAccumulator += dt;
  if (m_spawnAccumulator > (1.0f / spawnRate)) {
    m_spawnAccumulator = 0.0f;
    SpawnObject();
  }

  // CPU -> Gravity shake?
  // Actually standard gravity is fine.

  // Limit object count to avoid explosion
  if (m_objects.size() > 200) {
    // Remove oldest
    PhysicsObject &obj = m_objects[0];
    m_physicsWorld->GetWorld()->removeRigidBody(obj.body);
    delete obj.body->getMotionState();
    delete obj.body->getCollisionShape();
    delete obj.body;
    m_objects.erase(m_objects.begin());
  }
}

void GravityVisualizer::SpawnObject() {
  int type = rand() % 2; // 0 or 1

  btCollisionShape *colShape = nullptr;
  if (type == 0)
    colShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
  else
    colShape = new btSphereShape(0.5);

  btTransform startTransform;
  startTransform.setIdentity();

  // Random X/Z
  float x = ((rand() % 100) / 10.0f) - 5.0f;
  float z = ((rand() % 100) / 10.0f) - 5.0f;
  startTransform.setOrigin(btVector3(x, 10, z));

  btScalar mass(1.0f);
  btVector3 localInertia(0, 0, 0);
  colShape->calculateLocalInertia(mass, localInertia);

  btDefaultMotionState *myMotionState =
      new btDefaultMotionState(startTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape,
                                                  localInertia);
  btRigidBody *body = new btRigidBody(rbInfo);

  body->setRestitution(0.5f); // Bouncy

  m_physicsWorld->GetWorld()->addRigidBody(body);

  PhysicsObject obj;
  obj.body = body;
  obj.type = type;
  obj.color[0] = (rand() % 100) / 100.0f;
  obj.color[1] = (rand() % 100) / 100.0f;
  obj.color[2] = (rand() % 100) / 100.0f;

  m_objects.push_back(obj);
}

void GravityVisualizer::Draw(Shader *shader, const Mat4 &sceneTransform) {
  if (!IsEnabled() || !shader)
    return;

  shader->Use();

  for (const auto &obj : m_objects) {
    btTransform trans;
    if (obj.body && obj.body->getMotionState()) {
      obj.body->getMotionState()->getWorldTransform(trans);
    } else {
      continue;
    }

    // Convert btTransform to Mat4
    float m[16];
    trans.getOpenGLMatrix(m);

    // Manual conversion/copy if needed or just nice math
    Mat4 modelMat;
    for (int i = 0; i < 16; ++i)
      modelMat.m[i] = m[i];

    // Combine with scene
    Mat4 mvp = Mat4Multiply(sceneTransform, modelMat);

    shader->SetMat4("uModel", mvp.m.data());
    shader->SetVec3("uColor", obj.color[0], obj.color[1], obj.color[2]);

    if (obj.type == 0 && m_cubeMesh) {
      m_cubeMesh->Draw();
    } else if (obj.type == 1 && m_sphereMesh) {
      m_sphereMesh->Draw();
    }
  }
}

void GravityVisualizer::OnCleanup() {
  for (auto &obj : m_objects) {
    if (obj.body) {
      m_physicsWorld->GetWorld()->removeRigidBody(obj.body);
      delete obj.body->getMotionState();
      delete obj.body->getCollisionShape();
      delete obj.body;
    }
  }
  m_objects.clear();
}

bool GravityVisualizer::IsEnabled() const {
  return true; // TODO: Config
}
