#version 330 core

in vec3 vDirection;

layout(std140) uniform SceneData {
  mat4 uView;
  mat4 uProj;
  vec4 uLightDirAndAmbient;
  vec4 uCameraPos;
  vec4 uFogColor;
  vec4 uFogParams;
  vec4 uFogParams2;
};

uniform samplerCube uEnvironment;

out vec4 FragColor;

void main() {
  vec3 dir = normalize(vDirection);
  vec3 color = texture(uEnvironment, dir).rgb;
  FragColor = vec4(color, 1.0);
}
