#pragma once

#include "math/matrix.hpp"
#include "render/colors.hpp"
#include "render/program.hpp"

struct Cell {

  std::uint8_t bombsAround;

  struct VertexData {
    v3f color;
  };

  [[nodiscard]] constexpr VertexData getVertexData() const noexcept {
    return VertexData{.color = getColor()};
  }

private:
  [[nodiscard]] constexpr v3f getColor() const noexcept {
    switch (bombsAround) {
    case 0:
      return Color::White;
    case 1:
      return Color::Green;
    case 2:
      return Color::Yellow;
    case 3:
      return Color::Orange;
    case 4:
      return Color::Red;
    }
    logzy::critical("Unreachable reached.");
    std::unreachable();
  }
};

struct Board {

  void draw(const m4x4f &view, const m4x4f &projection) const;

  [[nodiscard]] static std::optional<Board> create(v3i dimensions);

private:
  /// Game data
  std::vector<Cell> cells_;

  /// Render data
  Program shaderProgram;

  GLuint vertexArray;

  GLuint cubeVertexBuffer;
  GLuint cellInstanceBuffer;
  GLuint texture;

  GLint vposLocation = -1;
  GLint voffsetLocation = -1;
  GLint vcolLocation = -1;
  GLint textureLocation = -1;
};
