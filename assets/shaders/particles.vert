#version 330 core

layout(location = 0) in vec3 aBase;
layout(location = 1) in vec3 aInstancePos;
layout(location = 2) in vec4 aInstanceColor;
layout(location = 3) in float aInstanceSize;

out vec4 vColor;

layout(std140) uniform SceneData {
  mat4 uView;
  mat4 uProj;
  vec4 uLightDirAndAmbient;
  vec4 uCameraPos;
  vec4 uFogColor;
  vec4 uFogParams;
  vec4 uFogParams2;
};

void main() {
  vec3 worldPos = aBase + aInstancePos;
  gl_Position = uProj * uView * vec4(worldPos, 1.0);
  gl_PointSize = aInstanceSize;
  vColor = aInstanceColor;
}
