#pragma once

#include <array>
#include <logzy/logzy.hpp>
#include <math.h>
#include <numbers>

struct v2 {
  float x;
  float y;
};

struct v3 {
  float x;
  float y;
  float z;
};

struct Vertex {
  v3 position;
  v3 color;
};

struct m4x4 {
  // Data of the matrxi stored in column-major order
  std::array<std::array<float, 4>, 4> data;

  [[nodiscard]] constexpr const float *dataPtr() const {
    return static_cast<const float *>(&data[0][0]);
  }

  [[nodiscard]] static constexpr m4x4 identity(float diagonalValue) {
    m4x4 identityMatrix;
    for (int i = 0; i < identityMatrix.data.size(); ++i) {
      identityMatrix.data[i][i] = diagonalValue;
    }

    return identityMatrix;
  }

  [[nodiscard]] constexpr m4x4 transposed() const noexcept {
    m4x4 columnMajor;

    for (int i = 0; i < data.size(); ++i) {
      for (int j = 0; j < data[i].size(); ++j) {
        columnMajor.data[j][i] = data[i][j];
      }
    }

    return columnMajor;
  }

  [[nodiscard]] constexpr m4x4 operator*(const m4x4 &other) const {
    m4x4 result;
    for (int i = 0; i < data.size(); ++i) {
      for (int j = 0; j < data[i].size(); ++j) {
        float sum = 0;
        for (int k = 0; k < data.size(); ++k) {
          sum += data[k][i] * other.data[j][k];
        }
        result.data[j][i] = sum;
      }
    }

    return result;
  }

  void print() const {
    logzy::info("m4x4\n{}\n{}\n{}\n{}", data[0], data[1], data[2], data[3]);
  }
};

[[nodiscard]] constexpr float toRadians(float degrees) {
  return degrees * std::numbers::pi / 180.0F;
}

[[nodiscard]] constexpr m4x4 orthographic(float left, float right, float bottom,
                                          float top, float near = 0.1F,
                                          float far = 100.0F) {

  return m4x4{

      .data = {std::array<float, 4>{2 / (right - left), 0.0F, 0.0F,
                                    -(right + left) / (right - left)}, // C1
               std::array<float, 4>{0.0F, 2.0F / (top - bottom), 0.0F,
                                    -(top + bottom) / (top - bottom)}, // C2
               std::array<float, 4>{0.0F, 0.0F, -2.0F / (far - near),
                                    -(far + near) / (far - near)}, // C3
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};     // C4
}

[[nodiscard]] constexpr m4x4 perspective(float fov = 50.0F, float near = 0.1F,
                                         float far = 100.0F) {
  float x = 1.0F / (toRadians(fov / 2.0F));

  return m4x4{
      .data = {
          std::array<float, 4>{x, 0.0F, 0.0F, 0.0F},
          std::array<float, 4>{0.0F, x, 0.0F, 0.0F},
          std::array<float, 4>{0.0F, 0.0F, -far / (far - near), -1.0F},
          std::array<float, 4>{0.0F, 0.0F, -far * near / (far - near), 0.0F}}};
}

[[nodiscard]] constexpr m4x4 translate(const m4x4 &mat, v3 vector) {

  m4x4 translationMatrix = {
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{vector.x, vector.y, vector.z, 1.0F}}};

  return mat * translationMatrix;
}

[[nodiscard]] constexpr m4x4 scale(const m4x4 &mat, v3 vector) {

  m4x4 scaleMatrix = {.data = {std::array<float, 4>{vector.x, 0.0F, 0.0F, 0.0F},
                               std::array<float, 4>{0.0F, vector.y, 0.0F, 0.0F},
                               std::array<float, 4>{0.0F, 0.0F, vector.z, 0.0F},
                               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return mat * scaleMatrix;
}

[[nodiscard]] constexpr m4x4 rotateX(const m4x4 &mat, float radians) {
  return m4x4{
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, cos(radians), -sin(radians), 0.0F},
               std::array<float, 4>{0.0F, sin(radians), cos(radians), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};
}
[[nodiscard]] constexpr m4x4 rotateY(const m4x4 &mat, float radians) {
  m4x4 rotationMatrix{
      .data = {std::array<float, 4>{cos(radians), 0.0F, sin(radians), 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{-sin(radians), 0.0F, cos(radians), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return mat * rotationMatrix;
}
[[nodiscard]] constexpr m4x4 rotateZ(const m4x4 &mat, float radians) {
  m4x4 rotationMatrix{
      .data = {std::array<float, 4>{cos(radians), -sin(radians), 0.0F, 0.0F},
               std::array<float, 4>{sin(radians), cos(radians), 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return mat * rotationMatrix;
}
