#pragma once

#include "../engine/Math.h"
#include "../glad/glad.h"
#include <vector>


// Mesh structure for OpenGL vertex data
struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  GLsizei indexCount = 0;
  GLsizei vertexCount = 0;
  bool indexed = false;
};

// Mesh creation functions
Mesh CreateCubeMesh();
Mesh CreateSphereMesh(int slices, int stacks);
Mesh CreateRingMesh(int segments, float innerRadius, float outerRadius);

// Mesh lifecycle
void DestroyMesh(Mesh &mesh);
void DrawMesh(const Mesh &mesh);
