#include "program.hpp"
#include "glad.h"
#include "logzy/logzy.hpp"

[[nodiscard]] std::optional<Program>
Program::create(const std::vector<std::pair<std::string_view, Shader::Type>>
                    &shaderSources) {

  logzy::debug("Creating program");

  std::optional<Program> p(Program{});
  p->ID = glCreateProgram();

  if (p->ID == 0) {
    logzy::error("Couldn't create shader program");
    return std::nullopt;
  }

  logzy::debug("Program {} created", p->ID);

  {
    // Shaders are auto-deleted
    std::vector<Shader> shaders;

    for (const auto &[shaderSource, type] : shaderSources) {
      std::optional<Shader> s = Shader::fromString(shaderSource, type);

      if (!s) {
        logzy::error("Couldn't create program - failed to create shader");
        return std::nullopt;
      }

      logzy::debug("Attaching shader {} to program {}", s->ID, p->ID);
      glAttachShader(p->ID, s->ID);

      shaders.emplace_back(std::move(*s));
    }

    logzy::debug("Linking program {} with {} shaders", p->ID, shaders.size());
    // Linking
    glLinkProgram(p->ID);
  }

  GLint success;
  // Checking link errors
  glGetProgramiv(p->ID, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    logzy::error("Program {} failed to link", p->ID);
    return std::nullopt;
  }

  logzy::debug("Program {} created", p->ID);
  return p;
}

void Program::use() const { glUseProgram(ID); }

std::optional<GLint>
Program::getUniformLocation(const std::string &name) const {
  // FIXME: all locations of uniforms could be cached if performance becomes a
  // problem.
  const GLint location = glGetUniformLocation(ID, name.c_str());

  if (location == -1) {
    logzy::error("Program {} doesn't contain uniform location {}", ID, name);
    return std::nullopt;
  }

  return std::optional(location);
}

std::optional<GLint> Program::getAttribLocation(const std::string &name) const {
  // FIXME: all locations of uniforms could be cached if performance becomes a
  // problem.
  const GLint location = glGetAttribLocation(ID, name.c_str());

  if (location == -1) {
    logzy::error("Program {} doesn't contain attribute location {}", ID, name);
    return std::nullopt;
  }

  return std::optional(location);
}

bool Program::setBool(const std::string &name, bool value) {

  auto locOpt = getUniformLocation(name);
  if (!locOpt) {
    return false;
  }

  glUniform1i(*locOpt, static_cast<GLint>(value));

  return true;
}

bool Program::setM4x4(const std::string &name, const m4x4f &matrix) {

  auto locOpt = getUniformLocation(name);
  if (!locOpt) {
    return false;
  }

  GLsizei count = 1;
  GLboolean transpose = GL_FALSE;
  glUniformMatrix4fv(*locOpt, count, transpose, dataPtrAs<GLfloat>(matrix));

  return true;
}
