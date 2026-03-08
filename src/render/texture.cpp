#include "texture.hpp"
#include "debug_utils.hpp"
#include "glad.h"

Texture::Texture(std::uint8_t *data, int width, int height,
                 TextureParams params) noexcept {
  DEBUG_ASSERT(data != nullptr);
  DEBUG_ASSERT(width > 0);
  DEBUG_ASSERT(height > 0);

  glGenTextures(1, &ID);
  glBindTexture(GL_TEXTURE_2D, ID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.horizontalWrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.verticalWrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magFilter);

  constexpr GLuint level = 0;
  // Docs say that it must be 0
  constexpr GLuint border = 0;

  glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, width, height, border, GL_RGB,
               GL_UNSIGNED_BYTE, data);
  if (params.genMipMap) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
}
