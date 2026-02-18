#include "crosshair.hpp"
#include "glad.h"
#include "math/math.hpp"
#include "render/mesh.hpp"
#include "render/program.hpp"
#include <stdexcept>

const char *vertexShader = R"""(
#version 330 core
in vec2 vPos;

uniform mat4 model;
uniform mat4 projection;

void main() {

   gl_Position = projection * model * vec4(vPos,0.0,1.0);
   
}
)""";
const char *fragmentShader = R"""(
#version 330 core

out vec4 Fragment;

uniform vec3 color;

void main() {
   Fragment = vec4(color,1.0);
}
)""";

Crosshair::Crosshair(v2u screenSize, v2u size, v3f color) {

  auto programOpt =
      Program::create({std::pair{vertexShader, Shader::Type::Vertex},
                       std::pair{fragmentShader, Shader::Type::Fragment}});

  if (!programOpt) {
    throw std::runtime_error("Shader failed to compile");
  }

  shaderProgram = std::move(*programOpt);
  // TODO :: Model will have to be chnaged more often when other hud element
  // come, But then i could organize the vertices with orthographic projection
  // and not SQUARE_VERTICES, or just calculate it here and upload the
  shaderProgram.use();
  shaderProgram.setM4x4(
      "model",
      scale(translate(identity<float, 4>(),
                      vec3(screenSize.x() * 0.5f, screenSize.y() * 0.5f, 0.0f)),
            vec3(size.x() * 1.0f, size.y() * 1.0f, 1.0f)));
  shaderProgram.setV3f("color", color);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  constexpr auto bufSizeBytes =
      SQUARE_VERTICES.size() * sizeof(SQUARE_VERTICES[0]);
  glBufferData(GL_ARRAY_BUFFER, bufSizeBytes, SQUARE_VERTICES.data(),
               GL_STATIC_DRAW);

  GLuint vPosLocation = 0;
  if (auto vPosOpt = programOpt->getAttribLocation("vPos")) {
    vPosLocation = *vPosOpt;
  }

  glEnableVertexAttribArray(vPosLocation);
  glVertexAttribPointer(vPosLocation, 2, GL_FLOAT, GL_FALSE, sizeof(v2f),
                        (void *)0);

  // Unbidning
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Crosshair::draw(const m4x4f &proj) const {

  shaderProgram.use();
  shaderProgram.setM4x4("projection", proj);
  glBindVertexArray(vao);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

Crosshair::~Crosshair() {
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}
