#pragma once

#include <array>
#include <logzy/logzy.hpp>
#include <math.h>
#include <numbers>

#include "matrix.hpp"

struct Vertex {
  v3f position;
  v3f color;
};

[[nodiscard]] constexpr float radians(float degrees) {
  return degrees * std::numbers::pi / 180.0F;
}

[[nodiscard]] constexpr m4x4f orthographic(float left, float right,
                                           float bottom, float top,
                                           float near = 0.1F,
                                           float far = 100.0F) {

  return m4x4f{

      .data = {std::array<float, 4>{2 / (right - left), 0.0F, 0.0F,
                                    -(right + left) / (right - left)}, // C1
               std::array<float, 4>{0.0F, 2.0F / (top - bottom), 0.0F,
                                    -(top + bottom) / (top - bottom)}, // C2
               std::array<float, 4>{0.0F, 0.0F, -2.0F / (far - near),
                                    -(far + near) / (far - near)}, // C3
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};     // C4
}

[[nodiscard]] constexpr m4x4f perspective(float fov = 50.0F, float near = 0.1F,
                                          float far = 100.0F) {
  float x = 1.0F / (radians(fov / 2.0F));

  return m4x4f{
      .data = {
          std::array<float, 4>{x, 0.0F, 0.0F, 0.0F},
          std::array<float, 4>{0.0F, x, 0.0F, 0.0F},
          std::array<float, 4>{0.0F, 0.0F, -far / (far - near), -1.0F},
          std::array<float, 4>{0.0F, 0.0F, -far * near / (far - near), 0.0F}}};
}

[[nodiscard]] constexpr m4x4f translate(const m4x4f &mat, const v3f vector) {

  m4x4f translationMatrix = {
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{vector.x(), vector.y(), vector.z(), 1.0F}}};

  return translationMatrix * mat;
}

[[nodiscard]] constexpr m4x4f scale(const m4x4f &mat, const v3f vector) {

  m4x4f scaleMatrix = {
      .data = {std::array<float, 4>{vector.x(), 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, vector.y(), 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, vector.z(), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return scaleMatrix * mat;
}

/**
 * Column-major version of the X-axis rotation matrix;
 */
[[nodiscard]] constexpr m4x4f rotateX(const m4x4f &mat, float radians) {

  m4x4f rotationMatrix = m4x4f{
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, cos(radians), sin(radians), 0.0F},
               std::array<float, 4>{0.0F, -sin(radians), cos(radians), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * mat;
}

/**
 * Column-major version of the Y-axis rotation matrix;
 */
[[nodiscard]] constexpr m4x4f rotateY(const m4x4f &mat, float radians) {
  m4x4f rotationMatrix{
      .data = {std::array<float, 4>{cos(radians), 0.0F, -sin(radians), 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{sin(radians), 0.0F, cos(radians), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * mat;
}

/**
 * Column-major version of the Y-axis rotation matrix;
 */
[[nodiscard]] constexpr m4x4f rotateZ(const m4x4f &mat, float radians) {
  m4x4f rotationMatrix{
      .data = {std::array<float, 4>{cos(radians), sin(radians), 0.0F, 0.0F},
               std::array<float, 4>{-sin(radians), cos(radians), 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * mat;
}

/**
 * Column-major rotation amtrix around axis {axis}
 */
[[nodiscard]] constexpr m4x4f rotate(const m4x4f &mat, float radians,
                                     v3f axis) {

  const float c = cos(radians);
  const float s = sin(radians);
  const float ic = 1.0F - c;

  const v3f normalized = normalize(axis);

  const float x = normalized.x();
  const float y = normalized.y();
  const float z = normalized.z();

  m4x4f rotationMatrix{.data = {std::array<float, 4>{                    //
                                                     c + x * x * ic,     //
                                                     x * y * ic + z * s, //
                                                     z * x * ic - y * s, //
                                                     0.0F},              //
                                std::array<float, 4>{
                                    x * y * ic - z * s, //
                                    c + y * y * ic,     //
                                    z * y * ic + x * s, //
                                    0.0F                //
                                },
                                std::array<float, 4>{
                                    x * z * ic + y * s, //
                                    y * z * ic - x * s, //
                                    c + z * z * ic,     //
                                    0.0F                //
                                },
                                std::array<float, 4>{
                                    0.0F,
                                    0.0F,
                                    0.0F,
                                    1.0F,
                                }}};

  return rotationMatrix * mat;
}

[[nodiscard]] constexpr m4x4f lookAt(const v3f cameraPosition, const v3f target,
                                     const v3f arbitraryUp) {

  v3f direction = normalize(target - cameraPosition);

  v3f right = cross(direction, arbitraryUp);
  v3f up = cross(right, direction);

  m4x4f positionMatrix{
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{-cameraPosition.x(), -cameraPosition.y(),
                                    -cameraPosition.z(), 1.0F}}};

  m4x4f rotationMatrix{
      .data = {std::array<float, 4>{right.x(), up.x(), direction.x(), 0.0F},
               std::array<float, 4>{right.y(), up.y(), direction.y(), 0.0F},
               std::array<float, 4>{right.z(), up.z(), direction.z(), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * positionMatrix;
}
