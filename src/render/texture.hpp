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
  int minFilter = GL_LINEAR;

  // Filtering
  int magFilter = GL_LINEAR;
};

struct Texture {

public:
  constexpr Texture() noexcept = default;
  // Creates a texture from given data. assumes the data is a valid buffer.
  // RGB only
  Texture(std::uint8_t *data, int width, int height,
          TextureParams params = {}) noexcept;
  void generateMipMaps();

public:
  GLuint ID = -1;
};

struct TextureArray {
public:
  constexpr TextureArray() noexcept = default;

  TextureArray(int width, int height, int layers,
               TextureParams params = {}) noexcept;

  void bind();

  // The texture must be bound before usage
  // All added textures should have the same size and height
  void addTexture(std::uint8_t *data, int layer);
  // The texture must be bound before usage
  void generateMipMaps();

public:
  GLuint ID = -1;

  int width = -1;
  int height = -1;
};
