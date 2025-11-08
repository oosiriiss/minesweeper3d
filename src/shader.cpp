#include "shader.hpp"
#include <logzy/logzy.hpp>
#include <optional>

[[nodiscard]] std::optional<Shader>
Shader::fromString(std::string_view shaderSource, Shader::Type type) {

  const GLuint openglShaderType = toOpenGL(type);

  // Creating the shader object
  std::optional<Shader> s({.ID = glCreateShader(openglShaderType)});

  if (s->ID == 0) {
    logzy::error("Couldn't create {} shader.", type);
    return std::nullopt;
  }

  // Compiling shader
  if (auto errorOpt = s->compile(shaderSource)) {
    logzy::error("Couldn't compile {} shader {}. OpenGL error: {}", type, s->ID,
                 *errorOpt);
    return std::nullopt;
  }
  logzy::info("{} shader {} created.", type, s->ID);

  return s;
}

std::optional<std::string> Shader::compile(std::string_view source) const {
  // Loading the shader data
  const char *sourceData = source.data();
  const GLint lengthArray = source.size();
  glShaderSource(ID, 1, &sourceData, &lengthArray);

  // Compiling shader
  glCompileShader(ID);

  // Checking for errors
  int success;
  std::array<char, 512> infoLog{};
  GLsizei outputLength = 0;

  glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    glGetShaderInfoLog(ID, infoLog.size(), &outputLength, infoLog.data());
    logzy::critical("Fragment shader didn't compile. {}", infoLog);
    std::optional<std::string> out(
        std::string(infoLog.begin(), std::next(infoLog.begin(), outputLength)));
    return out;
  };

  logzy::info("Shader compiled");

  return std::nullopt;
}

GLuint Shader::toOpenGL(Shader::Type type) {
  switch (type) {
  case Type::Vertex:
    return GL_VERTEX_SHADER;
  case Type::Fragment:
    return GL_FRAGMENT_SHADER;
    break;
  }
  logzy::error("Unreachable reached");
  std::unreachable();
}
