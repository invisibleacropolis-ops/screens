#version 330 core

in vec2 vTexCoord;

uniform sampler2D uHdrTexture;
uniform sampler2D uBloomTexture;
uniform float uExposure;
uniform float uBloomStrength;

out vec4 FragColor;

vec3 TonemapReinhard(vec3 color) {
  return color / (color + vec3(1.0));
}

void main() {
  vec3 hdrColor = texture(uHdrTexture, vTexCoord).rgb;
  vec3 bloom = texture(uBloomTexture, vTexCoord).rgb;
  vec3 color = hdrColor + bloom * uBloomStrength;
  color = vec3(1.0) - exp(-color * uExposure);
  color = TonemapReinhard(color);
  color = pow(color, vec3(1.0 / 2.2));
  FragColor = vec4(color, 1.0);
}
