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
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, 1.0f, -1.0f},
    {1.0f, 1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    {1.0f, 1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
    // Floor - Blue
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f},
    {1.0f, -1.0f, 1.0f},
    {-1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, 1.0f},
    // Ceil - Yellow
    {-1.0f, 1.0f, -1.0f},
    {-1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f}, 
    {-1.0f, 1.0f, -1.0f},
    {1.0f, 1.0f, -1.0f},
    {1.0f, 1.0f, 1.0f}, 
    // Left wall - Purple
    {-1.0f, -1.0f, 1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, 1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f},
    {-1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, -1.0f},
    // Right wall - White
    {1.0f, -1.0f, 1.0f},
    {1.0f, -1.0f, -1.0f},
    {1.0f, 1.0f, -1.0f},
    {1.0f, -1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f}, 
    {1.0f, 1.0f, -1.0f},

};
// clang-format on
