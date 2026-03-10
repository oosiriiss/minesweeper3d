#pragma once

#include <span>

#include "debug_utils.hpp"
#include "glad.h"
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
    v4f color;
    // Layer for sampler2DArray
    float textureIndex;
  };

  [[nodiscard]] constexpr v3f getColor() const noexcept;
  [[nodiscard]] constexpr float getTextureIndex() const noexcept;
};

struct Board {

public:
  static constexpr float CELL_SPACING = 0.1f;

  [[nodiscard]] static std::optional<Board> create(v3uz dimensions);
  void draw(const m4x4f &view, const m4x4f &projection) const;
  void dig(v3uz coords) noexcept;
  void flag(v3uz coords) noexcept;
  constexpr void changeCubeSize(float difference) noexcept;
  [[nodiscard]] constexpr v3f
  cellCenterPosition(v3uz cellCoords) const noexcept;

  void onLeftClick(v3f playerPos, v3f playerDir) noexcept;
  void onRightClick(v3f playerPos, v3f playerDir) noexcept;
  constexpr void toggleDrawNeighbours(bool draw) noexcept {
    drawDugAdjacent = draw;
    updateCubeInstanceData();
  }

private:
  /**
   * Returns the {x,y,z} indicating cell's position in internal cells_
   * vector[z][y][x]
   */
  [[nodiscard]] std::optional<v3uz>
  getPointedCell(v3f playerPos, v3f playerDir) const noexcept;

  // Game related methods
  [[nodiscard]] static std::vector<std::vector<std::vector<Cell>>>
  generateBoard(const v3uz dimensions, std::uint32_t bombs);

  // Render related methods
  void loadCubeMesh(const std::span<const v3f> mesh,
                    const std::span<const v2f> textureCoords);
  bool setupVAO(GLuint &vertexArrayID, GLuint &cellInstanceBufferID);
  void updateCubeInstanceData(v3uz pointedCellCoordiantes = vec3<size_t>(-1));

  /**
   *  Whether cube at [z][y][x] that have a undug bomb around them should be
   * visible (but partially seethrough)
   */
  [[nodiscard]] bool drawSeeThroughAdjacent(size_t x, size_t y,
                                            size_t z) const noexcept;

public:
  float cellSize = 1.0f;
  bool drawDugAdjacent = false;

private:
  /// Game data
  std::uint32_t bombsLeft = 0;

  // Board data
  // Accessed like: Board[z][y][x]
  // TODO :: Abstract this
  std::vector<std::vector<std::vector<Cell>>> cells_;

  /// Render data
  Program shaderProgram;

  // Cube vertex position buffer
  GLuint cubeMeshID;
  // Cube vertex texture coord buffer
  GLuint cubeUvID;

  GLuint opaqueVertexArrayID;
  GLuint opaqueCellInstanceBufferID;

  GLuint transparentVertexArrayID;
  GLuint transparentCellInstanceBufferID;

  size_t opaqueInstancesToDraw{0uz};
  size_t transparentInstancesToDraw{0uz};
};

constexpr v3f Cell::getColor() const noexcept {
  if (state == Cell::State::Flagged) {
    return Color::Purple;
  }

  if (state == Cell::State::Default) {
    return Color::Gray;
  }

  if (isBomb) {
    return Color::DarkGray;
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
  default: // bombs >= 4
    return Color::Red;
  }
  logzy::critical("Unreachable reached.");
  std::unreachable();
}

constexpr float Cell::getTextureIndex() const noexcept {
  constexpr float FLAG_LAYER_INDEX =
      27.0f; // float because its like this in shader
  constexpr float UNDUG_INDEX = 0.0f;
  if (state == Cell::State::Flagged) {
    return FLAG_LAYER_INDEX;
  }

  if (state == Cell::State::Default) {
    return UNDUG_INDEX;
  }
  DEBUG_ASSERT(bombsAround >= 0 && bombsAround <= 26,
               std::to_string(bombsAround));
  return static_cast<float>(bombsAround);
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
