#pragma once

#include <string>

#include "glad/glad.h"

class Shader {
public:
  Shader(const std::string &vertexPath, const std::string &fragmentPath);
  ~Shader();

  bool IsValid() const;
  void Use() const;
  void SetMat4(const std::string &name, const float *value) const;
  void SetVec3(const std::string &name, float x, float y, float z) const;
  void SetFloat(const std::string &name, float value) const;
  void SetInt(const std::string &name, int value) const;
  void BindUniformBlock(const std::string &name, GLuint binding) const;
  GLuint GetId() const { return programId_; }

private:
  GLuint programId_ = 0;

  static std::string LoadFile(const std::string &path);
  static GLuint CompileShader(GLenum type, const std::string &source);
  static void LogShaderError(GLuint id, bool isProgram,
                             const std::string &label);
};
