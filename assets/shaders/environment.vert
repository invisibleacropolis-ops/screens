#version 330 core

layout(location = 0) in vec3 aPos;

layout(std140) uniform SceneData {
  mat4 uView;
  mat4 uProj;
  vec4 uLightDirAndAmbient;
  vec4 uCameraPos;
  vec4 uFogColor;
  vec4 uFogParams;
  vec4 uFogParams2;
};

uniform float uScale;

out vec3 vDirection;

void main() {
  mat4 view = mat4(mat3(uView));
  vec4 worldPos = vec4(aPos * uScale, 1.0);
  vDirection = mat3(transpose(uView)) * aPos;
  gl_Position = uProj * view * worldPos;
}
