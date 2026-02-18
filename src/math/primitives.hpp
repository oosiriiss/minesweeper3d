#pragma once
#include "debug_utils.hpp"
#include "matrix.hpp"

/** 3D Ray that starts at the {origin} point and points in a given {direction}
 */
struct Ray {
  v3f origin;
  v3f direction;
};

/** 3D Axis Aligned Bounding Box */
struct AABB {
  /** Lowest coordinates along each axis. */
  v3f origin;
  /** Size along each axis */
  v3f size;

  /** Creates an AABB from element's center position and given size */
  [[nodiscard]] constexpr static auto fromCenterIn(v3f centerPosition,
                                                   v3f size) noexcept -> AABB {
    DEBUG_ASSERT(size.x() > 0 && size.y() > 0 && size.z() > 0);

    return AABB{.origin = centerPosition - (size * 0.5f), .size = size};
  }
};

