#pragma once

#include "glad.h"
#include <format>
#include <optional>
#include <string>
#include <string_view>

struct Shader {
  GLuint ID = 0;

  Shader() = default;
  Shader(GLuint ID);

  // Copying shader doesn't really make sense. As it is a temporary object
  Shader(const Shader &other) = delete;
  Shader &operator=(const Shader &other) = delete;
  // Moving is ok
  Shader(Shader &&other) noexcept;
  Shader &operator=(Shader &&other) noexcept;

  ~Shader();

  enum class Type { Vertex, Fragment };

  [[nodiscard]] static std::optional<Shader>
  fromString(std::string_view shaderCode, Shader::Type type);

private:
  /**
   * @brief compiles the shader
   *
   * @return std::nullopt on success and a string containing error msg on
   * failure.
   */
  std::optional<std::string> compile(std::string_view source) const;

  static GLuint toOpenGL(Shader::Type type);
};

template <> struct std::formatter<Shader::Type, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(Shader::Type type, FmtContext &ctx) const {

    std::string_view out;

    switch (type) {
    case Shader::Type::Vertex:
      out = "vertex";
      break;
    case Shader::Type::Fragment:
      out = "fragment";
      break;
    }

    return std::ranges::copy(out, ctx.out()).out;
  }
};
