#version 330 core

in vec2 vTexCoord;

uniform sampler2D uScene;
uniform float uThreshold;

out vec4 FragColor;

void main() {
  vec3 color = texture(uScene, vTexCoord).rgb;
  float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
  vec3 bright = luminance > uThreshold ? color : vec3(0.0);
  FragColor = vec4(bright, 1.0);
}
