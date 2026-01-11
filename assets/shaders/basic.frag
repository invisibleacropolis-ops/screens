#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uColor;

out vec4 FragColor;

void main() {
  vec3 normal = normalize(vNormal);
  // Simple directional light (top-right-front)
  vec3 lightDir = normalize(vec3(0.5, 1.0, 1.0));
  float diff = max(dot(normal, lightDir), 0.2); // 0.2 min ambient
  
  FragColor = vec4(uColor * diff, 1.0);
}
