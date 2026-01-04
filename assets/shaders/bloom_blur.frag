#version 330 core

in vec2 vTexCoord;

uniform sampler2D uImage;
uniform vec2 uTexelSize;
uniform int uHorizontal;

out vec4 FragColor;

void main() {
  float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
  vec2 offset = uHorizontal == 1 ? vec2(uTexelSize.x, 0.0) : vec2(0.0, uTexelSize.y);
  vec3 result = texture(uImage, vTexCoord).rgb * weights[0];

  for (int i = 1; i < 5; ++i) {
    vec2 delta = offset * float(i);
    result += texture(uImage, vTexCoord + delta).rgb * weights[i];
    result += texture(uImage, vTexCoord - delta).rgb * weights[i];
  }

  FragColor = vec4(result, 1.0);
}
