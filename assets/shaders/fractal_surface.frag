#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;
in float vHeight;

uniform vec3 uColor;
uniform float uTime;
uniform float uEnergy;
uniform float uPalettePhase;

out vec4 FragColor;

vec3 palette(float t) {
  vec3 a = vec3(0.55, 0.45, 0.50);
  vec3 b = vec3(0.45, 0.45, 0.40);
  vec3 c = vec3(1.00, 1.00, 1.00);
  vec3 d = vec3(0.00, 0.33, 0.67) + vec3(uPalettePhase * 0.01);
  return a + b * cos(6.28318 * (c * t + d));
}

void main() {
  vec3 n = normalize(vNormal);
  vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));
  vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0));

  float diff = max(dot(n, lightDir), 0.0);
  float hemi = 0.5 + 0.5 * n.y;
  float spec = pow(max(dot(reflect(-lightDir, n), viewDir), 0.0), 24.0);

  float band = sin(vHeight * 6.0 + uTime * 0.7 + uPalettePhase);
  float t = clamp(0.5 + 0.5 * band + uEnergy * 0.2, 0.0, 1.0);

  vec3 pal = palette(t);
  vec3 base = mix(uColor, pal, 0.65);

  float emissivePulse = 0.2 + 0.8 * smoothstep(0.2, 1.0, abs(band)) * uEnergy;
  vec3 color = base * (0.15 + diff * 0.7 + hemi * 0.15);
  color += pal * emissivePulse * 0.15;
  color += vec3(spec) * (0.2 + uEnergy * 0.3);

  FragColor = vec4(color, 1.0);
}
