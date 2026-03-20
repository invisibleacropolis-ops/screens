#include "WindVisualizer.h"
#include "../engine/Math.h"

WindVisualizer::WindVisualizer(const Config &config) : m_config(config) {}

WindVisualizer::~WindVisualizer() { Cleanup(); }

void WindVisualizer::OnInit() {
  // Create soft body cloth
  const btScalar s = 4.0f; // size
  const int resolution = 20;

  // Create a patch
  // (WorldInfo, Corner00, Corner10, Corner01, Corner11, ResX, ResY, Fixeds,
  // gendiags)
  m_softBody = btSoftBodyHelpers::CreatePatch(
      m_physicsWorld->GetSoftBodyWorldInfo(), btVector3(-s, s, 0),
      btVector3(s, s, 0), btVector3(-s, -s, 0), btVector3(s, -s, 0), resolution,
      resolution,
      1 + 2, // Fix corners 0 and 1 (top left, top right)
      true);

  // Material properties
  m_softBody->m_materials[0]->m_kVCF =
      1.0f; // Volume conservation coefficient [0,1]
  m_softBody->m_materials[0]->m_kDP = 0.005f; // Damping coefficient [0,1]
  m_softBody->m_materials[0]->m_kDG = 0.0f;   // Drag coefficient [0,1]
  m_softBody->m_materials[0]->m_kLF = 0.0f;   // Lift coefficient [0,1]
  m_softBody->m_materials[0]->m_kPR = 0.0f; // Pressure coefficient [-inf,+inf]
  m_softBody->m_materials[0]->m_kVC =
      0.0f; // Volume conservation coefficient [0,1]
  m_softBody->m_materials[0]->m_kDF =
      0.5f; // Dynamic friction coefficient [0,1]
  m_softBody->m_materials[0]->m_kMT = 0.05f; // Pose matching coefficient [0,1]
  m_softBody->m_materials[0]->m_kCHR = 1.0f; // Rigid contacts hardness [0,1]
  m_softBody->m_materials[0]->m_kKHR = 0.1f; // Kinetic contacts hardness [0,1]
  m_softBody->m_materials[0]->m_kSHR = 1.0f; // Soft contacts hardness [0,1]
  m_softBody->m_materials[0]->m_kAHR = 0.7f; // Anchors hardness [0,1]

  m_softBody->m_cfg.piterations = 2;
  m_softBody->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSidedLiftDrag;

  // Add to world
  m_physicsWorld->GetWorld()->addSoftBody(m_softBody);

  // Init GL buffers
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ibo);

  CreateCloth(resolution, resolution);
}

void WindVisualizer::CreateCloth(int resX, int resY) {
  // Just to init usage of indices if we were building manually
  // But actually we will just iterate faces from softbody or grid
  // The SoftBody has m_faces or m_nodes.
  // We used CreatePatch, so we know the topology.
  // Let's rely on CreatePatch topology: it creates a grid.
}

void WindVisualizer::OnUpdate(float dt, const SystemMonitor &monitor) {
  m_time += dt;

  if (!m_softBody)
    return;

  // Map CPU usage to Wind Velocity
  float cpu = static_cast<float>(monitor.GetCpuUsage()) / 100.0f;

  // Wind direction changes slowly with time (sin/cos)
  // Wind strength depends on CPU
  float windStrength = 5.0f + (cpu * 20.0f); // Base 5, max 25

  // Direction
  float angle = m_time * 0.5f;
  btVector3 windDir(std::sin(angle), 0.2f, std::cos(angle)); // Slightly upwards
  windDir.normalize();

  m_softBody->setWindVelocity(windDir * windStrength);

  // RAM usage -> Stiffness? Or Color?
  // Let's do Color in Draw.
}

void WindVisualizer::UpdateMeshFromSoftBody() {
  if (!m_softBody)
    return;

  // We need to rebuild vertices/indices or just update positions
  // SoftBody faces are stored in m_faces

  m_vertices.clear();
  m_indices.clear();

  const int numFaces = m_softBody->m_faces.size();

  // Simple flat shading or smooth shading?
  // Let's do simple triangle soup for now to avoid dealing with shared normals
  // logic manually unless we use links. Actually, nodes have normals if we
  // enable them.

  for (int i = 0; i < numFaces; ++i) {
    const btSoftBody::Face &face = m_softBody->m_faces[i];

    // 3 Nodes per face
    for (int j = 0; j < 3; ++j) {
      btSoftBody::Node *node = face.m_n[j];
      const btVector3 &pos = node->m_x;
      const btVector3 &norm = node->m_n;

      // Pos
      m_vertices.push_back(pos.x());
      m_vertices.push_back(pos.y());
      m_vertices.push_back(pos.z());

      // Normal
      m_vertices.push_back(norm.x());
      m_vertices.push_back(norm.y());
      m_vertices.push_back(norm.z());

      // UV (Fake it based on pos or node index? We don't have UVs in default
      // nodes easily unless we store them) Let's just use 0,0
      m_vertices.push_back(0.0f);
      m_vertices.push_back(0.0f);

      m_indices.push_back(m_vertices.size() / 8 - 1);
    }
  }

  // Upload to GPU
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float),
               m_vertices.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
               m_indices.data(), GL_DYNAMIC_DRAW);

  m_indexCount = m_indices.size();

  // Attrib pointers
  // Pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  // Norm
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  // UV
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));

  glBindVertexArray(0);
}

void WindVisualizer::Draw(Shader *shader, const Mat4 &sceneTransform) {
  if (!IsEnabled() || !shader)
    return;

  UpdateMeshFromSoftBody();

  shader->Use();

  // Model Matrix (Identity, simulation is in world space)
  // But we might want to scale it down if physics world is huge?
  // We used size 4 for cloth. View is usually looking at 0,0,0.

  Mat4 id = Mat4Identity();
  id = Mat4Multiply(sceneTransform, id);

  shader->SetMat4("uModel", id.m.data());

  // Color
  shader->SetVec3("uColor", 0.2f, 0.5f, 0.9f); // Blueish

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void WindVisualizer::OnCleanup() {
  if (m_vao) {
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
  }
  if (m_vbo) {
    glDeleteBuffers(1, &m_vbo);
    m_vbo = 0;
  }
  if (m_ibo) {
    glDeleteBuffers(1, &m_ibo);
    m_ibo = 0;
  }

  // PhysicsWorld handles softbody destruction via world cleanup usually,
  // but better to explicitly remove if we want to be safe before World
  // destruction. For now, PhysicsWorld destructor clears memory.
  m_softBody = nullptr;
}

bool WindVisualizer::IsEnabled() const {
  // Enabled via config "Wind" or part of "Physics"
  // For now, let's piggyback on existing or return true for testing
  // TODO: Add to Config
  return true;
}
