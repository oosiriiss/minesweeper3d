
#include "board.hpp"
#include "debug_utils.hpp"
#include "glad.h"
#include "math/intersections.hpp"
#include "math/math.hpp"
#include "render/mesh.hpp"
#include <cassert>
#include <logzy/logzy.hpp>

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

void Board::generateBoard(const v3u dimensions) {

  const std::size_t cellCount =
      dimensions.x() + dimensions.y() + dimensions.z();

  cells_.clear();
  cells_ = std::vector(
      dimensions.x(),
      std::vector(dimensions.y(), std::vector<Cell>(dimensions.z(), Cell{})));

  for (std::int32_t z = 0; z < dimensions.z(); ++z) {
    for (std::int32_t y = 0; y < dimensions.y(); ++y) {
      for (std::int32_t x = 0; x < dimensions.x(); ++x) {
        std::uint8_t bombs = z % 5;

        cells_[z][y][x] =
            (Cell{.bombsAround = static_cast<std::uint8_t>(x % 5)});
      }
    }
  }
}

void Board::dig(v3u coords) noexcept {
  cells_[coords.z()][coords.y()][coords.x()].isDug = true;
  // TODO :: (Performance)  Updating every cube, even those that didn't change
  updateCubeInstanceData();
}

void Board::testCollisions(v3f playerPos, v3f playerDir) {

  assert(cells_.size() > 0 && cells_[0].size() > 0 && cells_[0][0].size() > 0 &&
         "Board is generated and has least one cell.");

  std::vector<Cell::VertexData> instanceData;
  instanceData.reserve(cells_.size() * cells_[0].size() * cells_[0][0].size());

  for (std::size_t z = 0; z < cells_.size(); ++z) {
    for (std::size_t y = 0; y < cells_[z].size(); ++y) {
      for (std::size_t x = 0; x < cells_[z][y].size(); ++x) {
        const Cell &cell = cells_[z][y][x];

        if (cell.isDug) {
          continue;
        }

        v3f cellCenter = cellCenterPosition(vec3(x, y, z));

        Cell::VertexData vd{
            .positionOffset = cellCenter,
            .color =
                (doesIntersect(Ray{.origin = playerPos, .direction = playerDir},
                               AABB::fromCenterIn(cellCenter, vec3(cellSize))))
                    ? vec3(1.0f, 0.08f, 0.6f)
                    : cell.getColor()};

        DEBUG_ONLY(if (doesIntersect(
                           Ray{.origin = playerPos, .direction = playerDir},
                           AABB::fromCenterIn(cellCenter, vec3(cellSize)))) {
          logzy::debug("Intersecting with cube[{}][{}][{}], cube position: {}",
                       z, y, x, cellCenterPosition(vec3(x, y, z)));
        });

        instanceData.emplace_back(vd);
      }
    }
  }

  // Uploading the data to GPU
  const std::size_t instanceDataSizeBytes =
      instanceData.size() * sizeof(instanceData[0]);

  glBindBuffer(GL_ARRAY_BUFFER, cellInstanceBufferID);
  glBufferData(GL_ARRAY_BUFFER, instanceDataSizeBytes, instanceData.data(),
               GL_DYNAMIC_DRAW);
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

out vec4 fragment;

void main() {
   fragment = vec4(color,1.0);
};
)""";

[[nodiscard]] std::optional<Board> Board::create(const v3u dimensions) {

  std::optional<Board> board(Board{});

  board->loadCubeMesh(std::span{CUBE_VERTICES});
  board->generateBoard(dimensions);

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

void Board::updateCubeInstanceData() const {

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

        if (cell.isDug) {
          continue;
        }

        instanceData.emplace_back(Cell::VertexData{
            .positionOffset = cellCenterPosition(vec3(x, y, z)),
            .color = cell.getColor(),
        });
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
