#include "board.hpp"

#include <cassert>
#include <limits>
#include <logzy/logzy.hpp>
#include <ranges>

#include "debug_utils.hpp"
#include "game/resource_manager.hpp"
#include "glad.h"
#include "math/intersections.hpp"
#include "math/matrix.hpp"
#include "math/random.hpp"
#include "render/mesh.hpp"

void Board::draw(const m4x4f &view, const m4x4f &projection) const {

  DEBUG_ASSERT(cells_.size() > 0 && cells_[0].size() > 0 &&
                   cells_[0][0].size() > 0,
               "Board is generated and has least one cell.");

  shaderProgram.use();

  // TODO :: Cache the location of uniforms
  shaderProgram.setFloat("uCellSize", cellSize);
  shaderProgram.setM4x4("view", view);
  shaderProgram.setM4x4("projection", projection);
  shaderProgram.setInt("Texture", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(
      GL_TEXTURE_2D_ARRAY,
      ResourceManager::getTextureArray(ResourceManager::TileTexturesKey).ID);

  if (opaqueInstancesToDraw > 0) {
    glBindVertexArray(opaqueVertexArrayID);
    glDrawArraysInstanced(GL_TRIANGLES, 0, CUBE_VERTICES.size(),
                          opaqueInstancesToDraw);
  }

  if (transparentInstancesToDraw > 0) {
    // Transparent cubes are split into separate buffer to make the alpha
    // blending work correct and to avoid depth sorting.
    // TODO :: Is this "dirty"?
    glDepthMask(GL_FALSE);

    glBindVertexArray(transparentVertexArrayID);
    glDrawArraysInstanced(GL_TRIANGLES, 0, CUBE_VERTICES.size(),
                          transparentInstancesToDraw);

    glDepthMask(GL_TRUE);
  }
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
withinBoard(const std::vector<std::vector<std::vector<Cell>>> &cells, size_t x,
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

constexpr static std::string_view vertexShaderText = R"(
#version 330 core
in vec4 vCol;
in vec3 vPos;
in vec3 vOffset;
in vec2 vTexCoord;
in float vTexID;

uniform float uCellSize;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;

// UV coordiantes and texture ID
out vec3 TexData;

void main() {
   vec3 worldPos = (vPos * uCellSize) + vOffset;
    gl_Position = projection * view * vec4(worldPos,1.0);

    color = vCol;
    TexData = vec3(vTexCoord,vTexID);
}
)";

constexpr static std::string_view fragmentShaderText = R"""(
#version 330 core
in vec4 color;
in vec3 TexData; // UV + layer
out vec4 Fragment;

uniform sampler2DArray Texture;

void main() {
   // Fragment = texture(Texture, TexData) * vec4(1.0,1.0,color.x,1.0);
   Fragment = color * 0.2  + texture(Texture,TexData) * 0.8;
};
)""";

[[nodiscard]] std::optional<Board> Board::create(const v3uz dimensions) {

  std::optional<Board> board(Board{});

  board->loadCubeMesh(std::span{CUBE_VERTICES}, std::span{CUBE_UV});
  std::uint32_t bombs = 20;
  board->cells_ = generateBoard(dimensions, bombs);

  std::optional<Program> programOpt = Program::create(std::vector{
      std::pair{vertexShaderText, Shader::Type::Vertex},
      std::pair{fragmentShaderText, Shader::Type::Fragment},
  });

  if (!programOpt.has_value()) {
    logzy::critical("Failed to create shader program for board");
    return std::nullopt;
  }
  board->shaderProgram = std::move(*programOpt);

  if (!board->setupVAO(board->opaqueVertexArrayID,
                       board->opaqueCellInstanceBufferID)) {
    logzy::critical("couldn't setup VAO for board's opaque cubes");
    return std::nullopt;
  }

  if (!board->setupVAO(board->transparentVertexArrayID,
                       board->transparentCellInstanceBufferID)) {
    logzy::critical("couldn't setup VAO for board's transparent cubes");
    return std::nullopt;
  }
  board->updateCubeInstanceData();

  logzy::info("Board created");

  return board;
}

void Board::loadCubeMesh(const std::span<const v3f> mesh,
                         const std::span<const v2f> textureCoords) {

  DEBUG_ASSERT(mesh.size() == textureCoords.size());

  GLuint buffers[2];
  glGenBuffers(2, buffers);
  cubeMeshID = buffers[0];
  cubeUvID = buffers[1];

  glBindBuffer(GL_ARRAY_BUFFER, cubeMeshID);
  std::size_t meshSizeBytes = mesh.size_bytes();
  glBufferData(GL_ARRAY_BUFFER, meshSizeBytes, mesh.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, cubeUvID);
  std::size_t textureCoordsSizeBytes = textureCoords.size_bytes();
  glBufferData(GL_ARRAY_BUFFER, textureCoordsSizeBytes, textureCoords.data(),
               GL_STATIC_DRAW);
}

[[nodiscard]] bool Board::drawSeeThroughAdjacent(size_t x, size_t y,
                                                 size_t z) const noexcept {
  const auto &currentCell = cells_[z][y][x];
  if (!drawDugAdjacent || currentCell.state == Cell::State::Default ||
      currentCell.bombsAround == 0) {
    return false;
  }

  for (int dz = -1; dz < 2; ++dz) {
    for (int dy = -1; dy < 2; ++dy) {
      for (int dx = -1; dx < 2; ++dx) {
        if (dx == 0 && dy == 0 && dz == 0) [[unlikely]] {
          continue;
        }
        size_t newX = x + dx;
        size_t newY = y + dy;
        size_t newZ = z + dz;
        if (!withinBoard(cells_, newX, newY, newZ)) {
          continue;
        }
        const auto &adjacentCell = cells_[newZ][newY][newX];
        if (adjacentCell.state != Cell::State::Dug) {
          return true;
        }
      }
    }
  }

  return false;
}

void Board::updateCubeInstanceData(v3uz pointedCellCoordiantes) {

  assert(cells_.size() > 0 && cells_[0].size() > 0 && cells_[0][0].size() > 0 &&
         "Board is generated and has least one cell.");

  std::vector<Cell::VertexData> opaqueInstanceData;
  opaqueInstanceData.reserve(cells_.size() * cells_[0].size() *
                             cells_[0][0].size() / 2);

  std::vector<Cell::VertexData> transparentInstanceData;
  transparentInstanceData.reserve(cells_.size() * cells_[0].size() *
                                  cells_[0][0].size() / 2);

  for (std::size_t z = 0; z < cells_.size(); ++z) {
    for (std::size_t y = 0; y < cells_[z].size(); ++y) {
      for (std::size_t x = 0; x < cells_[z][y].size(); ++x) {
        const Cell &cell = cells_[z][y][x];

        bool drawSeeThrough = drawSeeThroughAdjacent(x, y, z);

        if (cell.state == Cell::State::Dug && !drawSeeThrough) {
          continue;
        }

        bool doHighlight = (vec3(x, y, z) == pointedCellCoordiantes) &&
                           cell.state != Cell::State::Flagged;
        constexpr auto highlightColor = vec3(1.0f, 0.08f, 0.6f);

        v3f color = (doHighlight) ? highlightColor : cell.getColor();
        float alpha = (drawSeeThrough) ? 0.2f : 1.0f;

        Cell::VertexData data{
            .positionOffset = cellCenterPosition(vec3(x, y, z)),
            .color = vec4(color.x(), color.y(), color.z(), alpha),
            .textureIndex = cell.getTextureIndex()};

        // Dug cube.
        if (drawSeeThrough) {
          transparentInstanceData.emplace_back(data);
        } else { // Normal cube "undug"
          opaqueInstanceData.emplace_back(data);
        }
      }
    }
  }

  transparentInstancesToDraw = transparentInstanceData.size();
  opaqueInstancesToDraw = opaqueInstanceData.size();

  logzy::info("Loaded {} cube opaque instaces and {} transparent instances",
              opaqueInstancesToDraw, transparentInstancesToDraw);

  // Uploading the data to GPU

  // Transparent
  const std::size_t transparentSizeBytes =
      transparentInstanceData.size() * sizeof(transparentInstanceData[0]);
  glBindBuffer(GL_ARRAY_BUFFER, transparentCellInstanceBufferID);
  glBufferData(GL_ARRAY_BUFFER, transparentSizeBytes,
               transparentInstanceData.data(), GL_DYNAMIC_DRAW);

  // Opaque
  const std::size_t opaqueSizeBytes =
      opaqueInstanceData.size() * sizeof(opaqueInstanceData[0]);
  glBindBuffer(GL_ARRAY_BUFFER, opaqueCellInstanceBufferID);
  glBufferData(GL_ARRAY_BUFFER, opaqueSizeBytes, opaqueInstanceData.data(),
               GL_DYNAMIC_DRAW);
}

bool Board::setupVAO(GLuint &vertexArrayID, GLuint &cellInstanceBufferID) {

  // For mesh vertices
  const char *positionAttributeName = "vPos";
  // Texture coords
  const char *textureCoordsAttributeName = "vTexCoord";

  // INSTANCE
  // Actual cube position
  const char *positionOffsetAttributeName = "vOffset";
  const char *colorAttributeName = "vCol";
  const char *textureIDAttributeName = "vTexID";

  GLint vposLocation = -1;
  GLint texCoordLocation = -1;

  GLint voffsetLocation = -1;
  GLint vcolLocation = -1;
  GLuint vTextureIDLocation = -1;

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

  if (auto texCoordLocationOpt =
          shaderProgram.getAttribLocation(textureCoordsAttributeName)) {
    texCoordLocation = *texCoordLocationOpt;
  } else {
    logzy::critical("Couldn't find {} attribute", textureCoordsAttributeName);
    return false;
  }

  if (auto textureIDLocationOpt =
          shaderProgram.getAttribLocation(textureIDAttributeName)) {
    vTextureIDLocation = *textureIDLocationOpt;
  } else {
    logzy::critical("Couldn't find {} attribute", textureIDAttributeName);
    return false;
  }
  logzy::info("{} shader attribute found", textureIDAttributeName);

  // Setting up VAO
  glGenVertexArrays(1, &vertexArrayID);
  glBindVertexArray(vertexArrayID);

  // cube mesh
  glBindBuffer(GL_ARRAY_BUFFER, cubeMeshID);
  glVertexAttribPointer(vposLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(CUBE_VERTICES[0]), nullptr);
  glEnableVertexAttribArray(vposLocation);

  // cube mesh coords
  glBindBuffer(GL_ARRAY_BUFFER, cubeUvID);
  glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE,
                        sizeof(CUBE_UV[0]), nullptr);
  glEnableVertexAttribArray(texCoordLocation);

  // instance buffers
  glGenBuffers(1, &cellInstanceBufferID);

  // Setting up instance data
  glBindBuffer(GL_ARRAY_BUFFER, cellInstanceBufferID);
  glVertexAttribPointer(voffsetLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Cell::VertexData),
                        (void *)offsetof(Cell::VertexData, positionOffset));
  glEnableVertexAttribArray(voffsetLocation);
  glVertexAttribDivisor(voffsetLocation, 1);

  glVertexAttribPointer(vcolLocation, 4, GL_FLOAT, GL_FALSE,
                        sizeof(Cell::VertexData),
                        (void *)offsetof(Cell::VertexData, color));
  glEnableVertexAttribArray(vcolLocation);
  glVertexAttribDivisor(vcolLocation, 1);

  glVertexAttribPointer(vTextureIDLocation, 1, GL_FLOAT, GL_FALSE,
                        sizeof(Cell::VertexData),
                        (void *)offsetof(Cell::VertexData, textureIndex));
  glEnableVertexAttribArray(vTextureIDLocation);
  glVertexAttribDivisor(vTextureIDLocation, 1);
  return true;
}
