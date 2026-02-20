#pragma once

#include <span>

#include "math/matrix.hpp"
#include "render/colors.hpp"
#include "render/program.hpp"

struct Cell {

  enum class State : std::uint8_t { Default, Dug, Flagged };

  std::uint8_t bombsAround{0};
  State state{State::Default};
  bool isBomb{false};

  struct VertexData {
    v3f positionOffset;
    v3f color;
  };

  [[nodiscard]] constexpr v3f getColor() const noexcept;
};

struct Board {

public:
  static constexpr float CELL_SPACING = 0.1f;

  [[nodiscard]] static std::optional<Board> create(v3u dimensions);
  void draw(const m4x4f &view, const m4x4f &projection) const;
  void dig(v3uz coords) noexcept;
  void flag(v3uz coords) noexcept;
  constexpr void changeCubeSize(float difference) noexcept;
  [[nodiscard]] constexpr v3f
  cellCenterPosition(v3uz cellCoords) const noexcept;

  void onLeftClick(v3f playerPos, v3f playerDir) noexcept;
  void onRightClick(v3f playerPos, v3f playerDir) noexcept;

private:
  /**
   * Returns the {x,y,z} indicating cell's position in internal cells_
   * vector[z][y][x]
   */
  [[nodiscard]] std::optional<v3uz>
  getPointedCell(v3f playerPos, v3f playerDir) const noexcept;

  // Game related methods
  void generateBoard(const v3u dimensions);

  // Render related methods
  void loadCubeMesh(const std::span<const v3f> mesh);
  bool setupVAO();
  void
  updateCubeInstanceData(v3uz pointedCellCoordiantes = vec3<size_t>(-1)) const;

public:
  float cellSize = 1.0f;

private:
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
  if (state == Cell::State::Flagged) {
    return Color::Purple;
  }

  switch (bombsAround) {
  case 0:
    return Color::Gray;
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

constexpr void Board::changeCubeSize(float difference) noexcept {
  cellSize += difference;
  updateCubeInstanceData();
}

[[nodiscard]] constexpr v3f
Board::cellCenterPosition(v3uz cellCoords) const noexcept {
  const float cellOffset = cellSize + CELL_SPACING;
  return vec3<float>(cellCoords.x() * cellOffset, cellCoords.y() * cellOffset,
                     cellCoords.z() * cellOffset);
}

#include <format>
#include <unordered_map>

template <> struct std::formatter<Cell::State, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(const Cell::State state, FmtContext &ctx) const {

    static std::unordered_map<Cell::State, const char *> mappings{
        {Cell::State::Default, "Default"},
        {Cell::State::Dug, "Dug"},
        {Cell::State::Flagged, "Flagged"},
    };

    return std::format_to(ctx.out(), "{}", mappings[state]);
  }
};
