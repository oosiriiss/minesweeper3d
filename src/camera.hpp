#pragma once

#include "math.hpp"

struct Camera {

  constexpr Camera(v3 initialPosition, v3 arbitraryUp)
      : position(initialPosition), up(arbitraryUp),
        right(direction.cross(arbitraryUp)), arbitraryUp(arbitraryUp) {
    updateDirection();
  }

  enum class Direction { Left, Right, Forward, Backward, Up, Down };

  // Rotation around X axis (up-down) in degrees.
  float pitch = 0.0F;
  // Rotation around Y axis (left-right) in degrees.
  float yaw = -90.0F;
  // Rotation around Z axis (rolling left-right) in degrees.
  float roll = 0.0F;

  // Camera world position
  v3 position{0.0F, 0.0F, 0.0F};

  // Direction the camera is looking at
  v3 direction{0.0F, 0.0F, -1.0F};

  // camera up vector
  v3 up{0.0F, 1.0F, 0.0F};

  // camera up vector
  v3 right{1.0F, 0.0F, 0.0F};

  // World up vector
  v3 arbitraryUp{0.0F, 1.0F, 0.0F};

  [[nodiscard]] constexpr m4x4 getView() {

    m4x4 positionMatrix{.data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
                                 std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
                                 std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
                                 std::array<float, 4>{-position.x, -position.y,
                                                      -position.z, 1.0F}}};

    m4x4 rotationMatrix{
        .data = {std::array<float, 4>{right.x, up.x, direction.x, 0.0F},
                 std::array<float, 4>{right.y, up.y, direction.y, 0.0F},
                 std::array<float, 4>{right.z, up.z, direction.z, 0.0F},
                 std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

    return rotationMatrix * positionMatrix;
  }

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
      position = position + this->direction * distance;
      break;
    case Direction::Backward:
      position = position - this->direction * distance;
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
  void rotate(v3 rotations) {

    constexpr auto MAX_PITCH = 89.9F;
    constexpr auto MIN_PITCH = -89.9F;
    // constexpr auto MAX_YAW = 360.0F;
    // constexpr auto MAX_ROLL = 360.0F;

    pitch = std::min(std::max(MIN_PITCH, pitch + rotations.x), MAX_PITCH);
    yaw += rotations.y;
    roll += rotations.z;
    // yaw = fmod((yaw + rotations.y), MAX_YAW);
    // roll = fmod((roll + rotations.z), MAX_ROLL);

    updateDirection();
  }

private:
  constexpr void updateDirection() {
    direction.x = cos(radians(yaw)) * cos(radians(pitch));
    direction.y = sin(radians(pitch));
    direction.z = sin(radians(yaw)) * cos(radians(pitch));

    direction = direction.normalize();

    right = direction.cross(arbitraryUp).normalize();
    up = right.cross(direction).normalize();
  }
};
