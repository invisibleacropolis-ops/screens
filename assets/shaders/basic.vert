#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

layout(std140) uniform SceneData {
  mat4 uView;
  mat4 uProj;
  vec3 uLightDir;
  float uAmbient;
};

uniform mat4 uModel;

out vec3 vNormal;

void main() {
  mat4 model = uModel;
  vec4 worldPos = model * vec4(aPos, 1.0);
  vNormal = mat3(transpose(inverse(model))) * aNormal;
  gl_Position = uProj * uView * worldPos;
}
