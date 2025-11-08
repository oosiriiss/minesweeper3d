#pragma once
#include "glad.h"
#include "math.hpp"
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

  bool setBool(const std::string &name, bool value);
  bool setM4x4(const std::string &name, const m4x4 &value);
};
