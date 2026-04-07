#pragma once

#include "math/matrix.hpp"
#include "render/program.hpp"

// TODO :: Rest of theHUD

struct Crosshair {

  Crosshair(v2u screenSize, v2u size, v3f color);
  ~Crosshair();

  void draw(const m4x4f &proj) const;

private:
  Program shaderProgram; // Later this would could be whole HUD shader

  GLuint projectionLoc_ = -1;
  GLuint vao;
  GLuint vbo;
};
