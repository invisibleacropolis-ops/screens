#version 330 core

in vec3 vNormal;

layout(std140) uniform SceneData {
  mat4 uView;
  mat4 uProj;
  vec3 uLightDir;
  float uAmbient;
};

uniform vec3 uColor;

out vec4 FragColor;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 lightDir = normalize(-uLightDir);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 color = uColor * (uAmbient + diff);
  FragColor = vec4(color, 1.0);
}
