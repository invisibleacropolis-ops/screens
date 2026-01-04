#include "Shader.h"

#include <fstream>
#include <sstream>
#include <windows.h>

namespace {
void LogMessage(const std::string &message) {
  OutputDebugStringA(message.c_str());
}
} // namespace

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) {
  std::string vertexSource = LoadFile(vertexPath);
  std::string fragmentSource = LoadFile(fragmentPath);

  if (vertexSource.empty() || fragmentSource.empty()) {
    LogMessage("Shader source missing or empty.\n");
    return;
  }

  GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
  GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

  if (!vertexShader || !fragmentShader) {
    if (vertexShader)
      glDeleteShader(vertexShader);
    if (fragmentShader)
      glDeleteShader(fragmentShader);
    return;
  }

  programId_ = glCreateProgram();
  glAttachShader(programId_, vertexShader);
  glAttachShader(programId_, fragmentShader);
  glLinkProgram(programId_);

  LogShaderError(programId_, true, "Program");

  GLint linked = GL_FALSE;
  glGetProgramiv(programId_, GL_LINK_STATUS, &linked);
  if (!linked) {
    glDeleteProgram(programId_);
    programId_ = 0;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader() {
  if (programId_) {
    glDeleteProgram(programId_);
  }
}

bool Shader::IsValid() const { return programId_ != 0; }

void Shader::Use() const { glUseProgram(programId_); }

void Shader::SetMat4(const std::string &name, const float *value) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  if (location >= 0) {
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
  }
}

void Shader::SetVec3(const std::string &name, float x, float y, float z) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  if (location >= 0) {
    glUniform3f(location, x, y, z);
  }
}

void Shader::SetFloat(const std::string &name, float value) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  if (location >= 0) {
    glUniform1f(location, value);
  }
}

void Shader::SetInt(const std::string &name, int value) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  if (location >= 0) {
    glUniform1i(location, value);
  }
}

void Shader::BindUniformBlock(const std::string &name, GLuint binding) const {
  GLuint index = glGetUniformBlockIndex(programId_, name.c_str());
  if (index != GL_INVALID_INDEX) {
    glUniformBlockBinding(programId_, index, binding);
  }
}

std::string Shader::LoadFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    LogMessage("Failed to open shader file: " + path + "\n");
    return {};
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

GLuint Shader::CompileShader(GLenum type, const std::string &source) {
  GLuint shader = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  LogShaderError(shader, false, type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");

  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

void Shader::LogShaderError(GLuint id, bool isProgram,
                            const std::string &label) {
  GLint status = GL_FALSE;
  GLint length = 0;
  if (isProgram) {
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
  } else {
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
  }

  if (length > 1) {
    std::string log(static_cast<size_t>(length), '\0');
    if (isProgram) {
      glGetProgramInfoLog(id, length, nullptr, log.data());
    } else {
      glGetShaderInfoLog(id, length, nullptr, log.data());
    }
    LogMessage(label + " shader log:\n" + log + "\n");
  }
}
