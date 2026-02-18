#pragma once

#include "math/matrix.hpp"

#include "glad.h"
#include "shader.hpp"
#include <vector>

struct Program {
  GLuint ID = 0;
  [[nodiscard]] static std::optional<Program>
  create(const std::vector<std::pair<std::string_view, Shader::Type>>
             &shaderSources);

  void use() const;

  std::optional<GLint> getUniformLocation(const std::string &name) const;
  std::optional<GLint> getAttribLocation(const std::string &name) const;

  bool setBool(const std::string &name, bool value) const;
  // Assumes location is valid
  void setBool(GLint location, bool value) const;

  bool setM4x4(const std::string &name, const m4x4f &matrix) const;
  // Assumes location is valid
  void setM4x4(GLint location, const m4x4f &matrix) const;

  bool setV3f(const std::string &name, v3f value) const;
  // Assumes location is valid
  void setV3f(GLint location, v3f value) const;

  bool setV4f(const std::string &name, v4f value) const;
  // Assumes location is valid
  void setV4f(GLint location, v4f value) const;

  bool setFloat(const std::string &name, float value) const;
  // Assumes location is valid
  void setFloat(GLint location, float value) const;
};
