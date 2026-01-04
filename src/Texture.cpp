#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <array>
#include <vector>

namespace {
struct FallbackFace {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

constexpr std::array<FallbackFace, 6> kFallbackFaces = {{{255, 90, 90},
                                                         {90, 255, 90},
                                                         {90, 90, 255},
                                                         {255, 255, 90},
                                                         {90, 255, 255},
                                                         {255, 90, 255}}};

GLenum ChooseFormat(int channels) {
  if (channels == 4) {
    return GL_RGBA;
  }
  if (channels == 3) {
    return GL_RGB;
  }
  return GL_RED;
}

GLint ChooseInternalFormat(int channels) {
  if (channels == 4) {
    return GL_SRGB8_ALPHA8;
  }
  if (channels == 3) {
    return GL_SRGB8;
  }
  return GL_R8;
}
} // namespace

GLuint Texture::LoadCubeMap(const std::array<std::string, 6> &faces,
                            bool flipVertical) {
  GLuint textureId = 0;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

  stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

  for (size_t i = 0; i < faces.size(); ++i) {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height,
                                    &channels, 0);
    if (!data) {
      const auto &fallback = kFallbackFaces[i];
      unsigned char fallbackPixel[3] = {fallback.r, fallback.g, fallback.b};
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0,
                   GL_SRGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                   fallbackPixel);
      continue;
    }

    GLenum format = ChooseFormat(channels);
    GLint internalFormat = ChooseInternalFormat(channels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0,
                 internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE,
                 data);
    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return textureId;
}
