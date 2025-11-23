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
    v3f{-1.0f, -1.0f, 1.0f},
    v3f{-1.0f, 1.0f, 1.0f},
    v3f{1.0f, 1.0f, 1.0f}, 
    v3f{-1.0f, -1.0f, 1.0f},
    v3f{1.0f, 1.0f, 1.0f}, 
    v3f{1.0f, -1.0f, 1.0f},
    // Back face of cube 
    v3f{-1.0f, -1.0f, -1.0f},
    v3f{-1.0f, 1.0f, -1.0f},
    v3f{1.0f, 1.0f, -1.0f},
    v3f{-1.0f, -1.0f, -1.0f},
    v3f{1.0f, 1.0f, -1.0f},
    v3f{1.0f, -1.0f, -1.0f},
    // Floor - Blue
    v3f{-1.0f, -1.0f, -1.0f},
    v3f{-1.0f, -1.0f, 1.0f},
    v3f{1.0f, -1.0f, 1.0f},
    v3f{-1.0f, -1.0f, -1.0f},
    v3f{1.0f, -1.0f, -1.0f},
    v3f{1.0f, -1.0f, 1.0f},
    // Ceil - Yellow
    v3f{-1.0f, 1.0f, -1.0f},
    v3f{-1.0f, 1.0f, 1.0f},
    v3f{1.0f, 1.0f, 1.0f}, 
    v3f{-1.0f, 1.0f, -1.0f},
    v3f{1.0f, 1.0f, -1.0f},
    v3f{1.0f, 1.0f, 1.0f}, 
    // Left wall - Purple
    v3f{-1.0f, -1.0f, 1.0f},
    v3f{-1.0f, -1.0f, -1.0f},
    v3f{-1.0f, 1.0f, -1.0f},
    v3f{-1.0f, -1.0f, 1.0f},
    v3f{-1.0f, 1.0f, 1.0f},
    v3f{-1.0f, 1.0f, -1.0f},
    // Right wall - White
    v3f{1.0f, -1.0f, 1.0f},
    v3f{1.0f, -1.0f, -1.0f},
    v3f{1.0f, 1.0f, -1.0f},
    v3f{1.0f, -1.0f, 1.0f},
    v3f{1.0f, 1.0f, 1.0f}, 
    v3f{1.0f, 1.0f, -1.0f},
};
// clang-format on
