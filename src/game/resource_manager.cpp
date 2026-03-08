#include "resource_manager.hpp"
#include "debug_utils.hpp"
#include "render/texture.hpp"
#include <filesystem>
#include <logzy/logzy.hpp>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

auto ResourceManager::loadTexture(std::string_view assetPath,
                                  TextureParams params,
                                  std::filesystem::path basePath) -> bool {

  std::filesystem::path fullPath = basePath / assetPath;
  DEBUG_ONLY(logzy::info("Loading texture at: '{}'", fullPath));

  if (!verifyPath(fullPath)) {
    return false;
  }

  int width = -1;
  int height = -1;
  int channels = -1;
  constexpr int desiredChannels = 3;

  auto *data = stbi_load(fullPath.string().c_str(), &width, &height, &channels,
                         desiredChannels);

  DEBUG_ASSERT(width > 0, std::format("Actual width: {}", width));
  DEBUG_ASSERT(height > 0, std::format("Actual width: {}", height));
  DEBUG_ASSERT(
      channels == desiredChannels,
      std::format("channels={} desired={}", channels, desiredChannels));

  Texture texture(data, width, height, TextureParams{.genMipMap = true});

  ResourceManager::textures_[fullPath.string()] =
      Texture{data, width, height, TextureParams{.genMipMap = true}};
  //
  stbi_image_free(data);
  return true;
}

auto ResourceManager::getTexture(std::string_view assetPath,
                                 std::filesystem::path basePath)
    -> const Texture & {

  std::filesystem::path fullPath = basePath / assetPath;

  std::string key = fullPath.string();

  auto it = ResourceManager::textures_.find(key);
  DEBUG_ONLY(if (it == ResourceManager::textures_.end()) {
    throw std::runtime_error(
        std::format("Trying to use texture '{}' that was not loaded", key));
  });

  return it->second;
}

auto ResourceManager::verifyPath(std::filesystem::path path) noexcept -> bool {
  if (!std::filesystem::exists(path)) {
    DEBUG_ONLY(logzy::warn("Path: '{}' doesn't exist", path.string()));
    return false;
  }

  return true;
}
