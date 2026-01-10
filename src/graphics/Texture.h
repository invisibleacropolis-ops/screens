#pragma once

#include <array>
#include <string>

#include "../glad/glad.h"

class Texture {
public:
  static GLuint LoadCubeMap(const std::array<std::string, 6> &faces,
                            bool flipVertical = false);
};
