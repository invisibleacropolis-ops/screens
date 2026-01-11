#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float opacity;

void main() {
    vec4 col = texture(screenTexture, TexCoords);
    FragColor = vec4(col.rgb, col.a * opacity);
}
