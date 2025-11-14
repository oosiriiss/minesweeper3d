#pragma once

#include <array>
#include <concepts>
#include <logzy/logzy.hpp>
#include <math.h>
#include <numeric>
#include <string>

// TODO :: MAtrix is row major but multiplication is done as column major
// TODO :: Allow multiplication of matrice with different sizes
// TODO :: iterator for matrix
// TODO :: Maybe separate the vector template class from mat template

template <typename T>
concept Arithmetic = requires(T a, T b) {
  { a + b } -> std::same_as<T>;
  { a - b } -> std::same_as<T>;
  { a / b } -> std::same_as<T>;
  { a * b } -> std::same_as<T>;
};

template <Arithmetic T, std::size_t Rows, std::size_t Cols> struct mat {
  /**
   * data[i][j] accesses element at row i and column j
   */
  std::array<std::array<T, Cols>, Rows> data{};
};

using m4x4f = mat<float, 4, 4>;
using v2f = mat<float, 1, 2>;
using v2d = mat<double, 1, 2>;
using v3f = mat<float, 1, 3>;

template <Arithmetic NewType, Arithmetic T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr mat<NewType, Rows, Cols>
castAs(const mat<T, Rows, Cols> &in) {

  static_assert(!std::same_as<T, NewType>,
                "Casting to the same type doesn't make  sense.");

  mat<NewType, Rows, Cols> out;

  for (std::size_t i = 0; i < Rows; ++i) {
    for (std::size_t j = 0; j < Cols; ++j) {
      out.data[i][j] = static_cast<NewType>(in.data[i][j]);
    }
  }

  return out;
}

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

template <Arithmetic T>
[[nodiscard]] constexpr mat<T, 1, 2> vec2(T x, T y, T z) {
  return mat<T, 1, 2>{.data = {{x, y}}};
}

template <Arithmetic T>
[[nodiscard]] mat<T, 1, 3> constexpr vec3(T x, T y, T z) {
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

  for (std::size_t i = 0; i < Cols; ++i) {
    for (std::size_t j = 0; j < Rows; ++j) {
      out.data[i][j] = m.data[j][i];
    }
  }

  return out;
}

// Matrix multiplication
template <Arithmetic T, std::size_t FirstRows, std::size_t FirstCols,
          std::size_t SecondCols>
[[nodiscard]] constexpr mat<T, FirstRows, SecondCols>
operator*(const mat<T, FirstRows, FirstCols> &f,
          const mat<T, FirstCols, SecondCols> &s) {
  mat<T, FirstRows, SecondCols> out;

  for (std::size_t i = 0; i < FirstRows; ++i) {
    for (std::size_t j = 0; j < SecondCols; ++j) {
      T sum = 0;
      for (std::size_t k = 0; k < FirstCols; ++k) {
        sum += f.data[i][k] * s.data[k][j];
      }

      out.data[i][j] = sum;
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
template <Arithmetic T, std::size_t Rows, std::size_t Cols>
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
template <Arithmetic T, std::size_t Rows, std::size_t Cols>
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
template <Arithmetic T, std::size_t Cols>
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

template <Arithmetic T>
[[nodiscard]] constexpr mat<T, 1, 3> cross(const mat<T, 1, 3> &first,
                                           const mat<T, 1, 3> &second) {

  return mat<T, 1, 3>{
      .data = {{first.y() * second.z() - first.z() * second.y(),
                first.z() * second.x() - first.x() * second.z(),
                first.x() * second.y() - first.y() * second.x()}}};
}

template <class Out, class T, std::size_t Rows, std::size_t Cols>
[[nodiscard]] constexpr const Out *dataAs(const mat<T, Rows, Cols> &m) {
  return static_cast<const Out *>(&(m.data[0][0]));
}

//////////////////////
//// Formatters //////
//////////////////////

#include <format>

template <class T, std::size_t Rows, std::size_t Cols>
struct std::formatter<mat<T, Rows, Cols>, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(const mat<T, Rows, Cols> &v,
                              FmtContext &ctx) const {

    // TODO :: Maybe this could be done better
    std::string out = "[\n";

    for (std::size_t i = 0; i < Rows; ++i) {
      out.append("  [");
      for (const auto &cell : v.data[i]) {
        out += std::to_string(cell);
        out.push_back(',');
      }
      out.append("]\n");
    };
    out.push_back(']');

    return std::format_to(ctx.out(), "mat<Rows={},Cols={}>({})", Rows, Cols,
                          out);
  }
};
