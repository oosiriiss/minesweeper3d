#pragma once
#include "glad.h"
#include <cstdint>

struct TextureParams {
  // Behaviour in case of texture 'S' (Horizontal) axis cooridantes are out of
  // bounds
  int horizontalWrap = GL_MIRRORED_REPEAT;
  // Behaviour in case of texture 'T' (Vertical) axis cooridantes are out of
  // bounds
  int verticalWrap = GL_MIRRORED_REPEAT;

  // Filtering
  int minFilter = GL_LINEAR_MIPMAP_LINEAR;

  // Filtering
  int magFilter = GL_LINEAR;
  bool genMipMap = 0;
};

struct Texture {

public:
  constexpr Texture() noexcept = default;
  // Creates a texture from given data. assumes the data is a valid buffer.
  Texture(std::uint8_t *data, int width, int height,
          TextureParams params = {}) noexcept;

public:
  GLuint ID = -1;
};
