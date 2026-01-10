#include "Mesh.h"

Mesh CreateCubeMesh() {
  Mesh mesh{};
  float vertices[] = {
      // positions          // normals
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
      0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,
      0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
      0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f,
      0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
      0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,
      0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
      0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,
      0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,

      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
      1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,
      1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
      -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
      -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f};

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glBindVertexArray(0);

  mesh.vertexCount =
      static_cast<GLsizei>(sizeof(vertices) / (6 * sizeof(float)));
  mesh.indexed = false;
  return mesh;
}

Mesh CreateSphereMesh(int slices, int stacks) {
  Mesh mesh{};
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  for (int stack = 0; stack <= stacks; ++stack) {
    float v = static_cast<float>(stack) / static_cast<float>(stacks);
    float phi = v * kPi;
    float cosPhi = std::cos(phi);
    float sinPhi = std::sin(phi);

    for (int slice = 0; slice <= slices; ++slice) {
      float u = static_cast<float>(slice) / static_cast<float>(slices);
      float theta = u * 2.0f * kPi;
      float cosTheta = std::cos(theta);
      float sinTheta = std::sin(theta);

      float x = sinPhi * cosTheta;
      float y = cosPhi;
      float z = sinPhi * sinTheta;

      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
    }
  }

  for (int stack = 0; stack < stacks; ++stack) {
    for (int slice = 0; slice < slices; ++slice) {
      int first = stack * (slices + 1) + slice;
      int second = first + slices + 1;

      indices.push_back(first);
      indices.push_back(second);
      indices.push_back(first + 1);
      indices.push_back(second);
      indices.push_back(second + 1);
      indices.push_back(first + 1);
    }
  }

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glBindVertexArray(0);

  mesh.indexCount = static_cast<GLsizei>(indices.size());
  mesh.indexed = true;
  return mesh;
}

Mesh CreateRingMesh(int segments, float innerRadius, float outerRadius) {
  Mesh mesh{};
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  for (int i = 0; i <= segments; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(segments);
    float angle = t * 2.0f * kPi;
    float c = std::cos(angle);
    float s = std::sin(angle);

    float innerX = innerRadius * c;
    float innerY = innerRadius * s;
    float outerX = outerRadius * c;
    float outerY = outerRadius * s;

    vertices.insert(vertices.end(), {innerX, innerY, 0.0f, 0.0f, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {outerX, outerY, 0.0f, 0.0f, 0.0f, 1.0f});
  }

  for (int i = 0; i < segments; ++i) {
    unsigned int start = i * 2;
    indices.push_back(start);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 1);
    indices.push_back(start + 3);
    indices.push_back(start + 2);
  }

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glBindVertexArray(0);

  mesh.indexCount = static_cast<GLsizei>(indices.size());
  mesh.indexed = true;
  return mesh;
}

void DestroyMesh(Mesh &mesh) {
  if (mesh.ebo) {
    glDeleteBuffers(1, &mesh.ebo);
  }
  if (mesh.vbo) {
    glDeleteBuffers(1, &mesh.vbo);
  }
  if (mesh.vao) {
    glDeleteVertexArrays(1, &mesh.vao);
  }
  mesh = {};
}

void DrawMesh(const Mesh &mesh) {
  glBindVertexArray(mesh.vao);
  if (mesh.indexed) {
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
  }
  glBindVertexArray(0);
}
