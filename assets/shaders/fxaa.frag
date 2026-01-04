#version 330 core

in vec2 vTexCoord;

uniform sampler2D uInput;
uniform vec2 uTexelSize;

out vec4 FragColor;

float Luma(vec3 color) {
  return dot(color, vec3(0.299, 0.587, 0.114));
}

void main() {
  vec3 rgbM = texture(uInput, vTexCoord).rgb;
  float lumaM = Luma(rgbM);

  vec3 rgbNW = texture(uInput, vTexCoord + vec2(-uTexelSize.x, uTexelSize.y)).rgb;
  vec3 rgbNE = texture(uInput, vTexCoord + vec2(uTexelSize.x, uTexelSize.y)).rgb;
  vec3 rgbSW = texture(uInput, vTexCoord + vec2(-uTexelSize.x, -uTexelSize.y)).rgb;
  vec3 rgbSE = texture(uInput, vTexCoord + vec2(uTexelSize.x, -uTexelSize.y)).rgb;

  float lumaNW = Luma(rgbNW);
  float lumaNE = Luma(rgbNE);
  float lumaSW = Luma(rgbSW);
  float lumaSE = Luma(rgbSE);

  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

  float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * 0.5, 1.0 / 128.0);
  float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
  dir = clamp(dir * rcpDirMin, vec2(-8.0), vec2(8.0)) * uTexelSize;

  vec3 rgbA = 0.5 * (texture(uInput, vTexCoord + dir * (1.0 / 3.0 - 0.5)).rgb +
                     texture(uInput, vTexCoord + dir * (2.0 / 3.0 - 0.5)).rgb);
  vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(uInput, vTexCoord + dir * -0.5).rgb +
                                  texture(uInput, vTexCoord + dir * 0.5).rgb);

  float lumaB = Luma(rgbB);
  vec3 result = (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;

  FragColor = vec4(result, 1.0);
}
