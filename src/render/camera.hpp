#pragma once

#include "math/math.hpp"
#include "math/matrix.hpp"
#include <logzy/logzy.hpp>

struct Camera {

  constexpr Camera(v3f initialPosition, v3f arbitraryUp)
      : position(initialPosition), up(arbitraryUp),
        right(cross(reverseDirection, arbitraryUp)), arbitraryUp(arbitraryUp) {
    updateDirection();
  }

  enum class Direction { Left, Right, Forward, Backward, Up, Down };

  // Rotation around X axis (up-down) in degrees.
  float pitch = 0.0F;
  // Rotation around Y axis (left-right) in degrees.
  float yaw = 90.0F;
  // Rotation around Z axis (rolling left-right) in degrees.
  float roll = 0.0F;

  // Camera world position
  v3f position = vec3<float>(0.0F, 0.0F, 0.0F);

  // Reverse of the direction the camera is looking at
  v3f reverseDirection = vec3<float>(0.0F, 0.0F, -1.0F);

  // camera up vector
  v3f up = vec3<float>(0.0F, 1.0F, 0.0F);

  // camera up vector
  v3f right = vec3<float>(1.0F, 0.0F, 0.0F);

  // World up vector
  v3f arbitraryUp = vec3<float>(0.0F, 1.0F, 0.0F);

  [[nodiscard]] constexpr m4x4f getView() {
    return lookAt(position, reverseDirection, up, right);
  }

  constexpr v3f getDirection() { return reverseDirection * -1; }

  /**
   * Moves the camera by the given delta position
   */
  void move(Direction direction, float distance) {

    switch (direction) {
    case Direction::Left:
      position = position + right * distance;
      break;
    case Direction::Right:
      position = position - right * distance;
      break;
    case Direction::Forward:
      position = position + this->reverseDirection * distance;
      break;
    case Direction::Backward:
      position = position - this->reverseDirection * distance;
      break;
    case Direction::Up:
      position = position - arbitraryUp * distance;
      break;
    case Direction::Down:
      position = position + arbitraryUp * distance;
      break;
    }
  }
  /**
   * Rotates the camera along each axis by given degrees.
   */
  void rotate(v3f rotations) {

    constexpr auto MAX_PITCH = 89.9F;
    constexpr auto MIN_PITCH = -89.9F;
    // constexpr auto MAX_YAW = 360.0F;
    // constexpr auto MAX_ROLL = 360.0F;

    pitch = std::min(std::max(MIN_PITCH, pitch + rotations.x()), MAX_PITCH);
    yaw += rotations.y();
    roll += rotations.z();
    // yaw = fmod((yaw + rotations.y), MAX_YAW);
    // roll = fmod((roll + rotations.z), MAX_ROLL);

    updateDirection();
  }

private:
  constexpr void updateDirection() {
    reverseDirection.x() = cos(radians(yaw)) * cos(radians(pitch));
    reverseDirection.y() = sin(radians(pitch));
    reverseDirection.z() = sin(radians(yaw)) * cos(radians(pitch));

    reverseDirection = normalize(reverseDirection);

    right = normalize(cross(reverseDirection, arbitraryUp));

    up = normalize(cross(right, reverseDirection));
  }
};
