#pragma once
#include "render/texture.hpp"
#include <filesystem>
#include <functional>
#include <string_view>
#include <unordered_map>

struct ResourceManager {

  inline static std::filesystem::path BASE_ASSET_PATH{"assets"};

  /**
   * @brief Loads texture into internal buffer
   * @param assetPath path to the asset relative to {basePath}
   * @param basePath base path of the asset, lower part of {assetPath}
   * @return true on success and False on failure
   */
  static auto loadTexture(std::string_view assetPath, TextureParams params = {},
                          std::filesystem::path basePath = BASE_ASSET_PATH)
      -> bool;

  [[nodiscard]] static auto
  getTexture(std::string_view assetPath,
             std::filesystem::path basePath = BASE_ASSET_PATH)
      -> const Texture &;

private:
  static auto verifyPath(std::filesystem::path path) noexcept -> bool;

  // Transparent lookup hash
  struct StringHash {
    using is_transparent = void;
    using hash_type = std::hash<std::string_view>;

    size_t operator()(const char *s) const { return hash_type{}(s); }
    size_t operator()(std::string_view s) const { return hash_type{}(s); }
    size_t operator()(const std::string &s) const { return hash_type{}(s); }
  };

  inline static std::unordered_map<std::string, Texture, StringHash,
                                   std::equal_to<>>
      textures_;
};
