#version 330 core

in vec4 vColor;

out vec4 FragColor;

void main() {
  vec2 coord = gl_PointCoord * 2.0 - 1.0;
  float dist = dot(coord, coord);
  float alpha = smoothstep(1.0, 0.2, dist);
  FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
