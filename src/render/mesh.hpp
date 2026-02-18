#pragma once

#include "math/matrix.hpp"

struct Vertex {
  v3f position;
  v3f color;
  v2f texture;
};

// clang-format off
constexpr std::array<v3f, 36> CUBE_VERTICES{
    // Front face of cube - Red
    v3f{-0.5f, -0.5f, 0.5f},
    v3f{-0.5f, 0.5f, 0.5f},
    v3f{0.5f, 0.5f, 0.5f}, 
    v3f{-0.5f, -0.5f, 0.5f},
    v3f{0.5f, 0.5f, 0.5f}, 
    v3f{0.5f, -0.5f, 0.5f},
    // Back face of cube 
    v3f{-0.5f, -0.5f, -0.5f},
    v3f{-0.5f, 0.5f, -0.5f},
    v3f{0.5f, 0.5f, -0.5f},
    v3f{-0.5f, -0.5f, -0.5f},
    v3f{0.5f, 0.5f, -0.5f},
    v3f{0.5f, -0.5f, -0.5f},
    // Floor - Blue
    // Assumes location is valid
    v3f{-0.5f, -0.5f, -0.5f},
    v3f{-0.5f, -0.5f, 0.5f},
    v3f{0.5f, -0.5f, 0.5f},
    v3f{-0.5f, -0.5f, -0.5f},
    v3f{0.5f, -0.5f, -0.5f},
    v3f{0.5f, -0.5f, 0.5f},
    // Ceil - Yellow
    v3f{-0.5f, 0.5f, -0.5f},
    v3f{-0.5f, 0.5f, 0.5f},
    v3f{0.5f, 0.5f, 0.5f}, 
    v3f{-0.5f, 0.5f, -0.5f},
    v3f{0.5f, 0.5f, -0.5f},
    v3f{0.5f, 0.5f, 0.5f}, 
    // Left wall - Purple
    v3f{-0.5f, -0.5f, 0.5f},
    v3f{-0.5f, -0.5f, -0.5f},
    v3f{-0.5f, 0.5f, -0.5f},
    v3f{-0.5f, -0.5f, 0.5f},
    v3f{-0.5f, 0.5f, 0.5f},
    v3f{-0.5f, 0.5f, -0.5f},
    // Right wall - White
    v3f{0.5f, -0.5f, 0.5f},
    v3f{0.5f, -0.5f, -0.5f},
    v3f{0.5f, 0.5f, -0.5f},
    v3f{0.5f, -0.5f, 0.5f},
    v3f{0.5f, 0.5f, 0.5f}, 
    v3f{0.5f, 0.5f, -0.5f},
};

constexpr std::array<v2f,6> SQUARE_VERTICES {
   v2f{-0.5f,-0.5f},
   v2f{-0.5f,0.5f},
   v2f{0.5f,0.5f},

   v2f{0.5f,0.5f},
   v2f{-0.5f,-0.5f},
   v2f{0.5f,-0.5f},
};

// clang-format on
