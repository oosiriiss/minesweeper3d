#pragma once

#include <span>

#include "math/matrix.hpp"
#include "render/colors.hpp"
#include "render/program.hpp"

struct Cell {

  std::uint8_t bombsAround{0};
  bool isDug{false};

  struct VertexData {
    v3f positionOffset;
    v3f color;
  };

  [[nodiscard]] constexpr v3f getColor() const noexcept;
};

struct Board {

  void draw(const m4x4f &view, const m4x4f &projection) const;

  [[nodiscard]] static std::optional<Board> create(v3u dimensions);

  void dig(v3u coords) noexcept;

private:
  // Game related methods
  void generateBoard(const v3u dimensions);

  // Render related methods
  void loadCubeMesh(const std::span<const v3f> mesh);
  bool setupVAO();
  void updateCubeInstanceData() const;

  /// Game data

  // Board data
  // Accessed like: Board[z][y][x]
  // TODO :: Maybe convert this 3D vector to a linear vector.
  std::vector<std::vector<std::vector<Cell>>> cells_;

  /// Render data
  Program shaderProgram;

  GLuint vertexArrayID;

  GLuint cubeMeshID;
  GLuint cellInstanceBufferID;
};

constexpr v3f Cell::getColor() const noexcept {
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
