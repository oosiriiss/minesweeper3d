#include "board.hpp"

#include <cassert>
#include <limits>
#include <logzy/logzy.hpp>
#include <ranges>

#include "debug_utils.hpp"
#include "glad.h"
#include "math/intersections.hpp"
#include "math/matrix.hpp"
#include "math/random.hpp"
#include "render/mesh.hpp"

void Board::draw(const m4x4f &view, const m4x4f &projection) const {

  shaderProgram.use();

  // TODO :: Cache the location of uniforms
  shaderProgram.setFloat("uCellSize", cellSize);
  shaderProgram.setM4x4("view", view);
  shaderProgram.setM4x4("projection", projection);

  glBindVertexArray(vertexArrayID);

  assert(cells_.size() > 0 && cells_[0].size() > 0 && cells_[0][0].size() > 0 &&
         "Board is generated and has least one cell.");

  // Assuming cells_ is a cube where each dimension has the same amount of
  // objects
  std::size_t instances =
      cells_.size() * cells_[0].size() * cells_[0][0].size();
  glDrawArraysInstanced(GL_TRIANGLES, 0, CUBE_VERTICES.size(), instances);
}

void Board::onLeftClick(v3f playerPos, v3f playerDir) noexcept {

  std::optional targetedCoordinates = getPointedCell(playerPos, playerDir);
  if (!targetedCoordinates) {
    return;
  }

  dig(*targetedCoordinates);
}

void Board::onRightClick(v3f playerPos, v3f playerDir) noexcept {
  std::optional targetedCoordinates = getPointedCell(playerPos, playerDir);
  if (!targetedCoordinates) {
    return;
  }

  flag(*targetedCoordinates);
}

[[nodiscard]] std::optional<v3uz>
Board::getPointedCell(v3f playerPos, v3f playerDir) const noexcept {

  assert(cells_.size() > 0 && cells_[0].size() > 0 && cells_[0][0].size() > 0 &&
         "Board is generated and has least one cell.");

  std::optional<v3uz> pointedCell;
  float minDistance = std::numeric_limits<float>::infinity();

  for (std::size_t z = 0; z < cells_.size(); ++z) {
    for (std::size_t y = 0; y < cells_[z].size(); ++y) {
      for (std::size_t x = 0; x < cells_[z][y].size(); ++x) {
        const Cell &cell = cells_[z][y][x];

        // Dug cells are "transparent"
        if (cell.state == Cell::State::Dug) {
          continue;
        }

        v3f cellCenter = cellCenterPosition(vec3(x, y, z));
        float currentDistance = length(cellCenter - playerPos);

        // Cell is further than closest, so dont bother checking if it
        // intersects
        if (pointedCell && currentDistance >= minDistance) {
          continue;
        }

        // Collision check
        if (!doesIntersect(Ray{.origin = playerPos, .direction = playerDir},
                           AABB::fromCenterIn(cellCenter, vec3(cellSize)))) {
          continue;
        }

        pointedCell.emplace(vec3(x, y, z));
        minDistance = currentDistance;
      }
    }
  }

  return pointedCell;
}

static constexpr bool
withinBoard(std::vector<std::vector<std::vector<Cell>>> &cells, size_t x,
            size_t y, size_t z) noexcept {
  // size_t is always >= 0, so skipping that check
  return x < cells[0][0].size() && y < cells[0].size() && z < cells.size();
}

constexpr static void
markBomb(std::vector<std::vector<std::vector<Cell>>> &cells, size_t x, size_t y,
         size_t z) noexcept {

  cells[z][y][x].isBomb = true;

  // Marking adjacent cells
  for (int dz = -1; dz < 2; ++dz) {
    for (int dy = -1; dy < 2; ++dy) {
      for (int dx = -1; dx < 2; ++dx) {
        if (dz == 0 && dy == 0 && dx == 0) [[unlikely]] {
          continue;
        }
        size_t newX = x + dx;
        size_t newY = y + dy;
        size_t newZ = z + dz;
        if (withinBoard(cells, newX, newY, newZ)) {
          ++cells[newZ][newY][newX].bombsAround;
        }
      }
    }
  }
}

[[nodiscard]] std::vector<std::vector<std::vector<Cell>>>
Board::generateBoard(const v3uz dimensions, std::uint32_t bombs) {
  logzy::debug("Creating board");

  // TODO :: ?Performance? check if size_t which is more than enough isn't
  // impacting performance because of its size.
  // TODO :: Performance Currently z*x*y could be really huge, and we really
  // only need a small fraction of that. There should be a better way to
  // generate board

  auto cells = std::vector(
      dimensions.x(),
      std::vector(dimensions.y(), std::vector<Cell>(dimensions.z(), Cell{})));

  const std::size_t cellCount =
      dimensions.x() * dimensions.y() * dimensions.z();

  logzy::debug("Generating random bomb positions");
  std::vector<size_t> bombIndices = randomUniqueRange(0uz, cellCount - 1);

  logzy::debug("Placing bombs {} at generated positions", bombs);

  logzy::debug("Bomb positions: {}", bombIndices);

  for (size_t index : bombIndices | std::views::take(bombs)) {
    size_t x = index % dimensions.x();
    size_t y = (index / dimensions.x()) % dimensions.y();
    size_t z = (index / (dimensions.x() * dimensions.y())) % dimensions.z();
    markBomb(cells, x, y, z);
  }
  logzy::debug("Board created");

  return cells;
}

static constexpr void digDFS(std::vector<std::vector<std::vector<Cell>>> &cells,
                             size_t x, size_t y, size_t z) noexcept {
  auto &cell = cells[z][y][x];
  DEBUG_ASSERT(cell.state == Cell::State::Default,
               "Passed coordiantes shouldn't be dug or flagged "
               "(==Cell::State::Default)");

  cells[z][y][x].state = Cell::State::Dug;

  if (cell.bombsAround > 0 || cell.isBomb) {
    return;
  }

  for (int dz = -1; dz < 2; ++dz) {
    for (int dy = -1; dy < 2; ++dy) {
      for (int dx = -1; dx < 2; ++dx) {
        if (dz == 0 && dy == 0 && dx == 0) [[unlikely]] {
          continue;
        }
        size_t newX = x + dx;
        size_t newY = y + dy;
        size_t newZ = z + dz;

        if (withinBoard(cells, newX, newY, newZ) &&
            cells[newZ][newY][newX].state == Cell::State::Default) {
          digDFS(cells, newX, newY, newZ);
        }
      }
    }
  }
}

void Board::dig(v3uz coords) noexcept {
  digDFS(cells_, coords.x(), coords.y(), coords.z());

  // TODO :: (Performance)  Updating every cube, even those that didn't
  // change
  updateCubeInstanceData();
}

void Board::flag(v3uz coords) noexcept {

  DEBUG_ASSERT(cells_[coords.z()][coords.y()][coords.x()].state !=
                   Cell::State::Dug,
               "Cell that is dug cannot be flagged");

  auto &cell = cells_[coords.z()][coords.y()][coords.x()];

  Cell::State newState = (cell.state == Cell::State::Flagged)
                             ? Cell::State::Default
                             : Cell::State::Flagged;

  cell.state = newState;
  logzy::debug("Changed flagged state of cell at: {} to: {}", coords, newState);

  // TODO :: (Performance)  Updating every cube, even those that didn't
  // change Flagging is really only related to one cube so there shoulnd't be
  // any problem to add this
  updateCubeInstanceData();
}

constexpr static std::string_view vertexShaderText = R"""(
#version 330 core
in vec3 vCol;
in vec3 vPos;
in vec3 vOffset;

uniform float uCellSize;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 color;

void main() {
   vec3 worldPos = (vPos * uCellSize) + vOffset;
    gl_Position = projection * view * vec4(worldPos,1.0);
    color = vCol;
};
)""";

constexpr static std::string_view fragmentShaderText = R"""(
#version 330 core
in vec3 color;

out vec4 Fragment;

void main() {
   Fragment = vec4(color,1.0);
};
)""";

[[nodiscard]] std::optional<Board> Board::create(const v3uz dimensions) {

  std::optional<Board> board(Board{});

  board->loadCubeMesh(std::span{CUBE_VERTICES});
  std::uint32_t bombs = 20;
  board->cells_ = generateBoard(dimensions, bombs);

  glGenBuffers(1, &board->cellInstanceBufferID);
  board->updateCubeInstanceData();

  std::optional<Program> programOpt = Program::create(std::vector{
      std::pair{vertexShaderText, Shader::Type::Vertex},
      std::pair{fragmentShaderText, Shader::Type::Fragment},
  });

  if (!programOpt.has_value()) {
    logzy::critical("Failed to create shader program for board");
    return std::nullopt;
  }
  board->shaderProgram = std::move(*programOpt);

  if (!board->setupVAO()) {
    logzy::critical("couldn't setup VAO for board");
    return std::nullopt;
  }

  logzy::info("Board created");

  return board;
}

void Board::loadCubeMesh(const std::span<const v3f> mesh) {
  glGenBuffers(1, &cubeMeshID);
  glBindBuffer(GL_ARRAY_BUFFER, cubeMeshID);
  std::size_t meshSizeBytes = mesh.size() * sizeof(mesh[0]);
  glBufferData(GL_ARRAY_BUFFER, meshSizeBytes, mesh.data(), GL_STATIC_DRAW);
}

void Board::updateCubeInstanceData(v3uz pointedCellCoordiantes) const {

  assert(cells_.size() > 0 && cells_[0].size() > 0 && cells_[0][0].size() > 0 &&
         "Board is generated and has least one cell.");

  std::vector<Cell::VertexData> instanceData;
  instanceData.reserve(cells_.size() * cells_[0].size() * cells_[0][0].size());

  logzy::info("Creating cube vertex data for {} instances",
              instanceData.capacity());

  for (std::size_t z = 0; z < cells_.size(); ++z) {
    for (std::size_t y = 0; y < cells_[z].size(); ++y) {
      for (std::size_t x = 0; x < cells_[z][y].size(); ++x) {
        const Cell &cell = cells_[z][y][x];

        if (cell.state == Cell::State::Dug) {
          continue;
        }

        // TODO :: Ia cell is dug but it has bombAround > 0 and it is next to a
        // cell that is not dug it should be still visible, maybe with low
        // alpha. OTherwise it would be imposible to win

        bool doHighlight = (vec3(x, y, z) == pointedCellCoordiantes) &&
                           cell.state != Cell::State::Flagged;
        constexpr auto highlightColor = vec3(1.0f, 0.08f, 0.6f);

        v3f color = (doHighlight) ? highlightColor : cell.getColor();

        DEBUG_ASSERT(cell.state != Cell::State::Dug,
                     "Shoulnd't add dug cube coordinates");
        instanceData.emplace_back(cellCenterPosition(vec3(x, y, z)), color);
      }
    }
  }

  logzy::info("Loaded {} cube instaces", instanceData.size());

  // Uploading the data to GPU
  const std::size_t instanceDataSizeBytes =
      instanceData.size() * sizeof(instanceData[0]);

  glBindBuffer(GL_ARRAY_BUFFER, cellInstanceBufferID);
  glBufferData(GL_ARRAY_BUFFER, instanceDataSizeBytes, instanceData.data(),
               GL_DYNAMIC_DRAW);
}

bool Board::setupVAO() {

  // For mesh vertices
  const char *positionAttributeName = "vPos";
  const char *colorAttributeName = "vCol";
  // Actual cube position
  const char *positionOffsetAttributeName = "vOffset";

  GLint vposLocation = -1;
  GLint voffsetLocation = -1;
  GLint vcolLocation = -1;

  if (auto vposLocationOpt =
          shaderProgram.getAttribLocation(positionAttributeName)) {
    vposLocation = *vposLocationOpt;
  } else {
    logzy::critical("Couldn't find {} attribute", positionAttributeName);
    return false;
  }
  logzy::info("{} shader attribute found", positionAttributeName);

  if (auto vcolLocationOpt =
          shaderProgram.getAttribLocation(colorAttributeName)) {
    vcolLocation = *vcolLocationOpt;
  } else {
    logzy::critical("Couldn't find {} attribute", colorAttributeName);
    return false;
  }
  logzy::info("{} shader attribute found", colorAttributeName);

  if (auto voffsetLocationOpt =
          shaderProgram.getAttribLocation(positionOffsetAttributeName)) {
    voffsetLocation = *voffsetLocationOpt;
  } else {
    logzy::critical("Couldn't find {} attribute", positionOffsetAttributeName);
    return false;
  }
  logzy::info("{} shader attribute found", positionOffsetAttributeName);

  // Setting up VAO
  glGenVertexArrays(1, &vertexArrayID);
  glBindVertexArray(vertexArrayID);

  // cube mesh
  glBindBuffer(GL_ARRAY_BUFFER, cubeMeshID);
  glVertexAttribPointer(vposLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(CUBE_VERTICES[0]), nullptr);
  glEnableVertexAttribArray(vposLocation);

  // Setting up instance data
  glBindBuffer(GL_ARRAY_BUFFER, cellInstanceBufferID);
  glVertexAttribPointer(voffsetLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Cell::VertexData),
                        (void *)offsetof(Cell::VertexData, positionOffset));
  glEnableVertexAttribArray(voffsetLocation);

  glVertexAttribPointer(vcolLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Cell::VertexData),
                        (void *)offsetof(Cell::VertexData, color));
  glEnableVertexAttribArray(vcolLocation);

  glVertexAttribDivisor(voffsetLocation, 1);
  glVertexAttribDivisor(vcolLocation, 1);
  return true;
}
