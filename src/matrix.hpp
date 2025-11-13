#pragma once

#include <array>
#include <math.h>
#include <numeric>
#include <string>

// TODO :: CastTo method that will cast vector to a vector of other numeric type
// TODO :: Generic templated vector
// TODO :: Allow multiplication of matrice with different sizes
// TODO :: std::formatter for new matrix type
// TODO :: iterator for matrix
// TODO :: Maybe separate the vector template class from mat template

template <class T>
concept Arithmetic = requires(const T a, const T b) {
  a + b;
  a - b;
  a * b;
  a / b;
};

template <Arithmetic T, std::size_t Rows, std::size_t Cols> struct mat {
  std::array<std::array<T, Cols>, Rows> data{};
};

template <Arithmetic T, std::size_t Size>
[[nodiscard]] constexpr mat<T, Size, Size> diagonal(T value) {

  mat<T, Size, Size> out;

  for (std::size_t i = 0; i < Size; ++i) {
    out.data[i][i] = value;
  }

  return out;
}

template <Arithmetic T, std::size_t Size>
[[nodiscard]] constexpr mat<T, Size, Size> identity() {
  return diagonal<T, Size>(static_cast<T>(1));
}

using v2f = mat<float, 1, 2>;
using v2d = mat<double, 1, 2>;
using v3f = mat<float, 1, 3>;

template <Arithmetic T> constexpr mat<T, 1, 2> vec2(T x, T y, T z) {
  return mat<T, 1, 2>{.data = {{x, y}}};
}

template <Arithmetic T> mat<T, 1, 3> constexpr vec3(T x, T y, T z) {
  return mat<T, 1, 3>{.data = {{x, y, z}}};
}

template <Arithmetic T> struct mat<T, 1, 3> {
  std::array<std::array<T, 3>, 1> data{};
  [[nodiscard]] constexpr T &x() { return data[0][0]; }
  [[nodiscard]] constexpr const T &x() const { return data[0][0]; }

  [[nodiscard]] constexpr T &y() { return data[0][1]; }
  [[nodiscard]] constexpr const T &y() const { return data[0][1]; }

  [[nodiscard]] constexpr T &z() { return data[0][2]; }
  [[nodiscard]] constexpr const T &z() const { return data[0][2]; }
};

// Transposition
template <Arithmetic T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Cols, Rows>
transpose(const mat<T, Rows, Cols> &m) {
  mat<T, Cols, Rows> out;

  for (std::size_t i = 0; i < Rows; ++i) {
    for (std::size_t j = 0; j < Cols; ++j) {
      out.data[j][i] = m[i][j];
    }
  }

  return out;
}

// Matrix multiplication
template <class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Rows, Cols>
operator*(const mat<T, Rows, Cols> &f, const mat<T, Rows, Cols> &s) {
  // TODO :: Finish this

  mat<T, Rows, Cols> out;

  for (int i = 0; i < f.data.size(); ++i) {
    for (int j = 0; j < f.data[i].size(); ++j) {
      float sum = 0;
      for (int k = 0; k < f.data.size(); ++k) {
        sum += f.data[k][i] * s.data[j][k];
      }
      out.data[j][i] = sum;
    }
  }

  return out;
}

// Scalar multiplication
template <Arithmetic Multiplier, class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<T, Rows, Cols>
operator*(const mat<T, Rows, Cols> &f, const Multiplier mult) {
  mat<T, Rows, Cols> out;

  for (std::size_t i = 0; i < f.data.size(); ++i) {
    for (std::size_t j = 0; j < f.data[i].size(); ++j) {
      out.data[i][j] = f.data[i][j] * mult;
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

  const T squareSum =
      std::accumulate(vec.data[0].begin(), vec.data[0].end(), static_cast<T>(0),
                      [](T sum, T number) { return sum + (number * number); });

  const T length = sqrt(squareSum);

  mat<T, 1, Cols> out;

  for (std::size_t i = 0; i < out.data[0].size(); ++i) {
    out.data[0][i] = vec.data[0][i] / length;
  }

  return out;
}

template <class T>
[[nodiscard]] constexpr mat<T, 1, 3> cross(const mat<T, 1, 3> &first,
                                           const mat<T, 1, 3> &second) {

  logzy::info("First: {}\nSecond:{}", first, second);

  logzy::info(" f x = {} y = {} z = {}", first.x(), first.y(), first.z());
  logzy::info(" s x = {} y = {} z = {}", second.x(), second.y(), second.z());

  return mat<T, 1, 3>{
      .data = {{first.y() * second.z() - first.z() * second.y(),
                first.z() * second.x() - first.x() * second.z(),
                first.x() * second.y() - first.y() * second.x()}}};
}

#include <format>

template <typename T, std::size_t Rows, std::size_t Cols>
struct std::formatter<mat<T, Rows, Cols>, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(mat<T, Rows, Cols> v, FmtContext &ctx) const {

    // TODO :: Maybe this could be done better
    std::string out;

    out.push_back('[');
    for (const auto &row : v.data) {
      out.push_back('[');
      for (const auto &cell : row) {
        out += std::to_string(cell);
        out.push_back(',');
      }
      out.push_back(']');
    };
    out.push_back(']');

    return std::format_to(ctx.out(), "mat<Rows={},Cols={}>({})", Rows, Cols,
                          out);
  }
};

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
