#pragma once

#include <array>
#include <logzy/logzy.hpp>
#include <math.h>
#include <numbers>

struct v2 {
  float x = 0.0f;
  float y = 0.0f;
};

struct v3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;

  [[nodiscard]] constexpr v3 normalize() const {
    const float length = sqrt(x * x + y * y + z * z);

    return v3{.x = x / length, .y = y / length, .z = z / length};
  }

  [[nodiscard]] constexpr v3 cross(const v3 o) const {
    return v3{
        .x = y * o.z - z * o.y,
        .y = z * o.x - x * o.z,
        .z = x * o.y - y * o.x,
    };
  }

  [[nodiscard]] constexpr v3 operator*(const float mult) const {
    return v3{.x = mult * x, .y = mult * y, .z = mult * z};
  }

  [[nodiscard]] constexpr v3 operator-(v3 other) const {
    return v3{.x = x - other.x, .y = y - other.y, .z = z - other.z};
  }

  [[nodiscard]] constexpr v3 operator+(v3 other) const {
    return v3{.x = x + other.x, .y = y + other.y, .z = z + other.z};
  }
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

  return translationMatrix * mat;
}

[[nodiscard]] constexpr m4x4 scale(const m4x4 &mat, v3 vector) {

  m4x4 scaleMatrix = {.data = {std::array<float, 4>{vector.x, 0.0F, 0.0F, 0.0F},
                               std::array<float, 4>{0.0F, vector.y, 0.0F, 0.0F},
                               std::array<float, 4>{0.0F, 0.0F, vector.z, 0.0F},
                               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return scaleMatrix * mat;
}

/**
 * Column-major version of the X-axis rotation matrix;
 */
[[nodiscard]] constexpr m4x4 rotateX(const m4x4 &mat, float radians) {

  m4x4 rotationMatrix = m4x4{
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, cos(radians), sin(radians), 0.0F},
               std::array<float, 4>{0.0F, -sin(radians), cos(radians), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * mat;
}

/**
 * Column-major version of the Y-axis rotation matrix;
 */
[[nodiscard]] constexpr m4x4 rotateY(const m4x4 &mat, float radians) {
  m4x4 rotationMatrix{
      .data = {std::array<float, 4>{cos(radians), 0.0F, -sin(radians), 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{sin(radians), 0.0F, cos(radians), 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * mat;
}

/**
 * Column-major version of the Y-axis rotation matrix;
 */
[[nodiscard]] constexpr m4x4 rotateZ(const m4x4 &mat, float radians) {
  m4x4 rotationMatrix{
      .data = {std::array<float, 4>{cos(radians), sin(radians), 0.0F, 0.0F},
               std::array<float, 4>{-sin(radians), cos(radians), 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * mat;
}

/**
 * Column-major rotation amtrix around axis {axis}
 */
[[nodiscard]] constexpr m4x4 rotate(const m4x4 &mat, float radians, v3 axis) {

  const float c = cos(radians);
  const float s = sin(radians);
  const float ic = 1.0F - c;

  const v3 normalized = axis.normalize();

  const float x = normalized.x;
  const float y = normalized.y;
  const float z = normalized.z;

  m4x4 rotationMatrix{.data = {std::array<float, 4>{                    //
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

[[nodiscard]] constexpr m4x4 lookAt(v3 cameraPosition, v3 target,
                                    v3 arbitraryUp) {

  v3 direction = (target - cameraPosition).normalize();

  v3 right = direction.cross(arbitraryUp);
  v3 up = right.cross(direction);

  m4x4 positionMatrix{
      .data = {std::array<float, 4>{1.0F, 0.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 1.0F, 0.0F, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 1.0F, 0.0F},
               std::array<float, 4>{-cameraPosition.x, -cameraPosition.y,
                                    -cameraPosition.z, 1.0F}}};

  m4x4 rotationMatrix{
      .data = {std::array<float, 4>{right.x, up.x, direction.x, 0.0F},
               std::array<float, 4>{right.y, up.y, direction.y, 0.0F},
               std::array<float, 4>{right.z, up.z, direction.z, 0.0F},
               std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F}}};

  return rotationMatrix * positionMatrix;
}

template <> struct std::formatter<v2, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(v2 v, FmtContext &ctx) const {

    return std::format_to(ctx.out(), "v2({},{})", v.x, v.y);
  }
};

template <> struct std::formatter<v3, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(v3 v, FmtContext &ctx) const {

    return std::format_to(ctx.out(), "v3({},{},{})", v.x, v.y, v.z);
  }
};

template <> struct std::formatter<m4x4, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(m4x4 m, FmtContext &ctx) const {

    return std::format_to(ctx.out(), "m4x4 (Column-major) (\n{}\n{}\n{}\n{})\n",
                          m.data[0], m.data[1], m.data[2], m.data[3]);
  }
};
