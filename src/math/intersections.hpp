#pragma once

#include "debug_utils.hpp"
#include "primitives.hpp"
#include <cmath>
#include <limits>
#include <logzy/logzy.hpp>

[[nodiscard]] constexpr static bool doesIntersect(Ray ray, AABB box) noexcept {

  float tmin = 0.0;
  float tmax = std::numeric_limits<float>::infinity();

  constexpr size_t dimensions = 3;

  for (size_t dim{0uz}; dim < dimensions; ++dim) {
    float dirRe = 1.0f / ray.direction.data[0][dim];

    float tMinHit = (box.origin.data[0][dim] - ray.origin.data[0][dim]) * dirRe;
    float tMaxHit = (box.origin.data[0][dim] + box.size.data[0][dim] -
                     ray.origin.data[0][dim]) *
                    dirRe;

    if (tMinHit > tMaxHit) {
      std::swap(tMinHit, tMaxHit);
    }

    tmin = std::max(tmin, tMinHit);
    tmax = std::min(tmax, tMaxHit);

    if (tmin > tmax) {
      return false;
    }
  }
  return true;
}
