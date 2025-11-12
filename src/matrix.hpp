#pragma once

#include <array>
#include <math.h>
#include <numeric>

// TODO :: CastTo method that will cast vector to a vector of other numeric type
// TODO :: Generic templated vector
// TODO :: Allow multiplication of matrice with different sizes
// TODO :: std::formatter for new matrix type
// TODO :: iterator for matrix

template <class T>
concept Arithmetic = requires(const T a, const T b) {
  a + b;
  a - b;
  a * b;
  a / b;
};

template <Arithmetic T, std::size_t Rows, std::size_t Cols> struct mat {

  std::array<std::array<T, Cols>, Rows> data;

  [[nodiscard]] constexpr mat<T, Rows, Cols>
  operator*(const mat<T, Rows, Cols> &other) {

    mat<T, Rows, Cols> out;
    out[0][0] = other.data[0][0];
    out[1][0] = data[0][0];
    return out;
  }
};

using v2f = mat<float, 1, 2>;
using v2d = mat<double, 1, 2>;
using v3f = mat<float, 1, 3>;

template <Arithmetic T> struct mat<T, 1, 3> {
  std::array<std::array<T, 3>, 1> data;
  [[nodiscard]] constexpr T &x() { return data[0]; }
  [[nodiscard]] constexpr const T &x() const { return data[0][0]; }

  [[nodiscard]] constexpr T &y() { return data[1]; }
  [[nodiscard]] constexpr const T &y() const { return data[0][0]; }

  [[nodiscard]] constexpr T &z() { return data[2]; }
  [[nodiscard]] constexpr const T &z() const { return data[0][0]; }
};

// Matrix multiplication
template <class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Rows, Cols>
operator*(const mat<T, Rows, Cols> &f, const mat<T, Rows, Cols> &s) {

  // TODO ::

  mat<T, Rows, Cols> out;
  out.data[0][0] = f.data[0][0];
  out.data[1][0] = s.data[0][0];
  return out;
}

// Scalar multiplication
template <Arithmetic Multiplier, class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Rows, Cols>
operator*(const mat<T, Rows, Cols> &f, const Multiplier mult) {
  mat<T, Rows, Cols> out;

  for (std::size_t i = 0; i < f.data.size(); ++i) {
    for (std::size_t j = 0; j < f.data[i].size(); ++j) {
      out[i][j] = f.data[i][j] * mult;
    }
  }

  return out;
}

// Matrix addition
template <class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Rows, Cols>
operator+(const mat<T, Rows, Cols> &f, const mat<T, Rows, Cols> &s) {
  mat<T, Rows, Cols> out;

  for (std::size_t i = 0; i < f.data.size(); ++i) {
    for (std::size_t j = 0; j < f.data[i].size(); ++j) {
      out.data[i][j] = f.data[i][j] + s.data[i][j];
    }
  }

  return out;
}

// Matrix substraction
template <class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Rows, Cols>
operator-(const mat<T, Rows, Cols> &f, const mat<T, Rows, Cols> &s) {
  mat<T, Rows, Cols> out;

  for (std::size_t i = 0; i < f.data.size(); ++i) {
    for (std::size_t j = 0; j < f.data[i].size(); ++j) {
      out.data[i][j] = f.data[i][j] - s.data[i][j];
    }
  }

  return out;
}

///////////////////////
// Vector operations //
///////////////////////

/**
 * Normalizes the vector
 */
template <class T, std::size_t Cols>
[[nodiscard]] constexpr mat<T, 1, Cols> normalize(const mat<T, 1, Cols> &vec) {

  const float squareSum =
      std::accumulate(vec.data[0].begin(), vec.data[0].end(), 0,
                      [](T sum, T number) { return sum + (number * number); });

  const float length = sqrt(squareSum);

  mat<T, 1, Cols> out;

  for (std::size_t i = 0; i < out.data.size(); ++i) {
    out.data[0][i] = vec.data[0][i] / length;
  }

  return out;
}

template <class T>
[[nodiscard]] constexpr mat<T, 1, 3> cross(const mat<T, 1, 3> &first,
                                           const mat<T, 1, 3> &second) {
  mat<T, 1, 3> out;

  out.x() = first.y() * second.z() - first.z() * second.y();
  out.y() = first.z() * second.x() - first.x() * second.z();
  out.z() = first.x() * second.y() - first.y() * second.x();

  return out;
}

// template <typename T> struct std::formatter<v2<T>, char> {
//
//   template <class ParseContext>
//   constexpr ParseContext::iterator parse(ParseContext &ctx) {
//     return ctx.begin();
//   }
//
//   template <class FmtContext>
//   FmtContext::iterator format(v2<T> v, FmtContext &ctx) const {
//
//     return std::format_to(ctx.out(), "v2({},{})", v.x, v.y);
//   }
// };
//
// template <> struct std::formatter<v3, char> {
//
//   template <class ParseContext>
//   constexpr ParseContext::iterator parse(ParseContext &ctx) {
//     return ctx.begin();
//   }
//
//   template <class FmtContext>
//   FmtContext::iterator format(v3 v, FmtContext &ctx) const {
//
//     return std::format_to(ctx.out(), "v3({},{},{})", v.x, v.y, v.z);
//   }
// };
//
// template <> struct std::formatter<m4x4, char> {
//
//   template <class ParseContext>
//   constexpr ParseContext::iterator parse(ParseContext &ctx) {
//     return ctx.begin();
//   }
//
//   template <class FmtContext>
//   FmtContext::iterator format(m4x4 m, FmtContext &ctx) const {
//
//     return std::format_to(ctx.out(), "m4x4 (Column-major)
//     (\n{}\n{}\n{}\n{})\n",
//                           m.data[0], m.data[1], m.data[2], m.data[3]);
//   }
// };
//
