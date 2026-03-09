#include "texture.hpp"
#include "debug_utils.hpp"
#include "glad.h"
#include <logzy/logzy.hpp>
#include <utility>

static constexpr GLuint toBinding(GLuint type, std::string_view expr) {
  switch (type) {
  case GL_TEXTURE_2D:
    return GL_TEXTURE_BINDING_2D;
  case GL_TEXTURE_2D_ARRAY:
    return GL_TEXTURE_BINDING_2D_ARRAY;
  default:
    std::unreachable();
  }

  std::unreachable();
}

#define ASSERT_BOUND(type, id)                                                 \
  DEBUG_ONLY({                                                                 \
    GLint I;                                                                   \
    glGetIntegerv(toBinding(type, #type), &I);                                 \
    DEBUG_ASSERT(I == id)                                                      \
  });

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
}

void Texture::generateMipMaps() {
  ASSERT_BOUND(GL_TEXTURE_2D, ID);
  glGenerateMipmap(GL_TEXTURE_2D);
}

TextureArray::TextureArray(int width, int height, int layers,
                           TextureParams params) noexcept
    : width{width}, height{height} {
  DEBUG_ASSERT(width > 0);
  DEBUG_ASSERT(height > 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &ID);
  glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, layers, 0, GL_RGB,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,
                  params.horizontalWrap);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, params.verticalWrap);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, params.minFilter);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, params.magFilter);
}

void TextureArray::bind() { glBindTexture(GL_TEXTURE_2D_ARRAY, ID); }

void TextureArray::addTexture(std::uint8_t *data, int layer) {
  ASSERT_BOUND(GL_TEXTURE_2D_ARRAY, ID);
  DEBUG_ASSERT(data != nullptr);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, GL_RGB,
                  GL_UNSIGNED_BYTE, data);

  // TODO

  DEBUG_ONLY(logzy::info("Texture at layer {} added", layer));
}

void TextureArray::generateMipMaps() {
  ASSERT_BOUND(GL_TEXTURE_2D_ARRAY, ID);
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}
