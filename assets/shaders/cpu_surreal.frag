#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;
in float vHeight;

uniform vec3 uColor;
uniform float uTime;
uniform float uEnergy;
uniform float uBurst;
uniform float uPalettePhase;

out vec4 FragColor;

vec3 palette(float t) {
  vec3 a = vec3(0.50, 0.45, 0.55);
  vec3 b = vec3(0.45, 0.35, 0.40);
  vec3 c = vec3(1.00, 1.00, 1.00);
  vec3 d = vec3(0.65, 0.20, 0.85) + vec3(uPalettePhase * 0.012);
  return a + b * cos(6.28318 * (c * t + d));
}

void main() {
  vec3 n = normalize(vNormal);
  vec3 lightDir = normalize(vec3(0.45, 1.0, 0.6));
  vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0));

  float diff = max(dot(n, lightDir), 0.0);
  float hemi = 0.5 + 0.5 * n.y;
  float rim = pow(1.0 - max(dot(n, viewDir), 0.0), 2.0);
  float spec = pow(max(dot(reflect(-lightDir, n), viewDir), 0.0), 24.0);

  float wave = sin(vHeight * 5.0 + uTime * 0.6 + uPalettePhase * 0.8);
  float t = clamp(0.45 + 0.35 * wave + 0.35 * uEnergy + 0.25 * uBurst, 0.0, 1.0);

  vec3 pal = palette(t);
  vec3 base = mix(uColor, pal, 0.7);

  float pulse = smoothstep(0.2, 1.0, abs(wave)) * (0.4 + 0.6 * uBurst);
  vec3 color = base * (0.12 + 0.68 * diff + 0.20 * hemi);
  color += pal * pulse * 0.22;
  color += vec3(0.35, 0.55, 1.0) * rim * (0.12 + 0.18 * uEnergy);
  color += vec3(spec) * (0.18 + 0.25 * uBurst);

  FragColor = vec4(color, 1.0);
}
