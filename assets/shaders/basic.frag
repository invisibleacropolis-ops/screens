#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

layout(std140) uniform SceneData {
  mat4 uView;
  mat4 uProj;
  vec4 uLightDirAndAmbient;
  vec4 uCameraPos;
  vec4 uFogColor;
  vec4 uFogParams;
  vec4 uFogParams2;
};

uniform vec3 uColor;
uniform samplerCube uEnvironmentMap;

out vec4 FragColor;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 lightDir = normalize(-uLightDirAndAmbient.xyz);
  float diff = max(dot(normal, lightDir), 0.0);
  float ambient = uLightDirAndAmbient.w;

  vec3 envSample = texture(uEnvironmentMap, normal).rgb;
  float envIntensity = uFogParams2.y;

  vec3 color = uColor * (ambient + diff) + envSample * envIntensity;

  vec3 cameraPos = uCameraPos.xyz;
  float distanceToCamera = length(cameraPos - vWorldPos);
  float heightFactor = exp(-uFogParams.y * (vWorldPos.y - uFogParams.z));
  float fogAmount = 1.0 - exp(-uFogParams.x * distanceToCamera * heightFactor);

  float shaftAlignment = max(dot(normalize(cameraPos - vWorldPos), lightDir), 0.0);
  float shaftFactor = 1.0 + uFogParams.w * pow(shaftAlignment, uFogParams2.x);
  fogAmount = clamp(fogAmount * shaftFactor, 0.0, 1.0);

  vec3 fogged = mix(color, uFogColor.rgb, fogAmount);
  FragColor = vec4(fogged, 1.0);
}
