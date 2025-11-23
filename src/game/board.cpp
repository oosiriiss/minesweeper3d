
#include "board.hpp"
#include "glad.h"
#include "math/math.hpp"
#include "render/mesh.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Board::draw(const m4x4f &view, const m4x4f &projection) const {

  shaderProgram.use();
  // Cache the location of uniforms
  shaderProgram.setM4x4(
      "model", scale(identity<float, 4>(), vec3<float>(0.07f, 0.07f, 0.07f)));

  shaderProgram.setM4x4("view", view);
  shaderProgram.setM4x4("projection", projection);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(vertexArray);

  GLuint instances = 125;
  glDrawArraysInstanced(GL_TRIANGLES, 0, CUBE_VERTICES.size(), instances);
}

constexpr static std::string_view vertexShaderText = R"""(
#version 330 core
in vec3 vCol;
in vec3 vPos;
in vec3 vOffset;
in vec2 vTextureCoordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 color;
out vec2 textureCoordinate;

void main() {
    gl_Position = projection * view * model * vec4(vPos + vOffset, 1.0);
    color = vCol;
    textureCoordinate = vTextureCoordinate;
};
)""";

constexpr static std::string_view fragmentShaderText = R"""(
#version 330 core
in vec3 color;
in vec2 textureCoordinate;

uniform sampler2D tex;

out vec4 fragment;


void main() {
   fragment = texture(tex,textureCoordinate) * vec4(color,1.0);
};
)""";

[[nodiscard]] std::optional<Board> Board::create(const v3i dimensions) {

  std::optional<Board> board(Board{});

  glGenBuffers(1, &board->cubeVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, board->cubeVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, CUBE_VERTICES.size() * sizeof(CUBE_VERTICES[0]),
               CUBE_VERTICES.data(), GL_STATIC_DRAW);

  // Generating cube offsets
  std::size_t instancesCount = static_cast<std::size_t>(
      dimensions.x() * dimensions.y() * dimensions.z());

  std::vector<Vertex> instances;
  instances.reserve(instancesCount);

  float spacing = 3.0f;

  for (int x = 0; x < dimensions.x(); ++x) {
    for (int y = 0; y < dimensions.y(); ++y) {
      for (int z = 0; z < dimensions.z(); ++z) {
        instances.push_back(Vertex{
            .position = vec3<float>(spacing * x, spacing * y, spacing * z),
            .color = Color::Red,
            .texture = {0.0F, 0.0F}});
      }
    }
  }

  glGenBuffers(1, &board->cellInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, board->cellInstanceBuffer);
  glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(instances[0]),
               instances.data(), GL_DYNAMIC_DRAW);

  v2i textureDim;
  std::int32_t channels;
  const char *texturePath{"assets/container.jpg"};

  // Making sure the texture isnt upside down
  stbi_set_flip_vertically_on_load(true);

  unsigned char *textureData =
      stbi_load(texturePath, &textureDim.x(), &textureDim.y(), &channels, 0);

  if (textureData == nullptr) {
    logzy::critical(
        "Couldn't find file: {}. Make sure it is in the executable's directory",
        texturePath);
    return std::nullopt;
  }

  logzy::info("Loaded texture. Dimensions: {}x{} and channels: {}",
              textureDim.x(), textureDim.y(), channels);

  glGenTextures(1, &board->texture);
  glBindTexture(GL_TEXTURE_2D, board->texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureDim.x(), textureDim.y(), 0,
               GL_RGB, GL_UNSIGNED_BYTE, textureData);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Freeing the image data as it is already loaded onto GPU.
  stbi_image_free(textureData);

  std::optional<Program> programOpt = Program::create(std::vector{
      std::pair{vertexShaderText, Shader::Type::Vertex},
      std::pair{fragmentShaderText, Shader::Type::Fragment},
  });

  board->shaderProgram = std::move(*programOpt);

  const Program &program = board->shaderProgram;

  if (auto vposLocationOpt = program.getAttribLocation("vPos")) {
    board->vposLocation = *vposLocationOpt;
  } else {
    return std::nullopt;
  }
  logzy::info("vposLocation found");
  if (auto vcolLocationOpt = program.getAttribLocation("vCol")) {
    board->vcolLocation = *vcolLocationOpt;
  } else {
    return std::nullopt;
  }
  logzy::info("vcolLocation found");
  if (auto voffsetLocationOpt = program.getAttribLocation("vOffset")) {
    board->voffsetLocation = *voffsetLocationOpt;
  } else {
    return std::nullopt;
  }

  logzy::info("voffsetLocation found");
  if (auto textureLocationOpt =

          program.getAttribLocation("vTextureCoordinate")) {
    board->textureLocation = *textureLocationOpt;
  } else {
    return std::nullopt;
  }

  logzy::info("vTextureCoordiante found");
  glGenVertexArrays(1, &board->vertexArray);

  glBindVertexArray(board->vertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, board->cubeVertexBuffer);

  glVertexAttribPointer(board->vposLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(CUBE_VERTICES[0]), nullptr);
  glEnableVertexAttribArray(board->vposLocation);

  // Setting up instances
  glBindBuffer(GL_ARRAY_BUFFER, board->cellInstanceBuffer);
  glEnableVertexAttribArray(board->voffsetLocation);
  glVertexAttribPointer(board->voffsetLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), offsetof(Vertex, position));

  glVertexAttribPointer(board->vcolLocation, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), (void *)offsetof(Vertex, color));
  glEnableVertexAttribArray(board->vcolLocation);

  glVertexAttribPointer(board->textureLocation, 2, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), (void *)offsetof(Vertex, texture));
  glEnableVertexAttribArray(board->textureLocation);

  glVertexAttribDivisor(board->voffsetLocation, 1);
  glVertexAttribDivisor(board->vcolLocation, 1);
  glVertexAttribDivisor(board->textureLocation, 1);

  return board;
}
