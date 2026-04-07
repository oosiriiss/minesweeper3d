#pragma once

#include "debug_utils.hpp"
#include "math/matrix.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <stdint.h>
#include <utility>

enum class KeyState : std::uint8_t {
  // Key is up
  Idle = 0,
  // Key was idle and got just pressed
  Pressed = 1,
  // Key was pressed on previous frame and is still pressed
  Held = 2,
  // Key was pressed and just got released
  Released = 3
};

enum class Key : std::uint8_t {
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  Space,
  LeftControl,
  LeftAlt,
  Escape,
  F1,
  __SizeGuard,
  Unknown,
};

using GLFWKeyCode = int;
using GLFWKeyState = int;
using GLFWMouseButton = int;

[[nodiscard]] constexpr GLFWKeyCode keyToGLFW(Key k) noexcept {
  static constexpr auto map = []() {
    std::array<GLFWKeyCode, static_cast<size_t>(Key::__SizeGuard)> arr{};
    arr.fill(GLFW_KEY_UNKNOWN);

    arr[static_cast<size_t>(Key::A)] = GLFW_KEY_A;
    arr[static_cast<size_t>(Key::B)] = GLFW_KEY_B;
    arr[static_cast<size_t>(Key::C)] = GLFW_KEY_C;
    arr[static_cast<size_t>(Key::D)] = GLFW_KEY_D;
    arr[static_cast<size_t>(Key::E)] = GLFW_KEY_E;
    arr[static_cast<size_t>(Key::F)] = GLFW_KEY_F;
    arr[static_cast<size_t>(Key::G)] = GLFW_KEY_G;
    arr[static_cast<size_t>(Key::H)] = GLFW_KEY_H;
    arr[static_cast<size_t>(Key::I)] = GLFW_KEY_I;
    arr[static_cast<size_t>(Key::J)] = GLFW_KEY_J;
    arr[static_cast<size_t>(Key::K)] = GLFW_KEY_K;
    arr[static_cast<size_t>(Key::L)] = GLFW_KEY_L;
    arr[static_cast<size_t>(Key::M)] = GLFW_KEY_M;
    arr[static_cast<size_t>(Key::N)] = GLFW_KEY_N;
    arr[static_cast<size_t>(Key::O)] = GLFW_KEY_O;
    arr[static_cast<size_t>(Key::P)] = GLFW_KEY_P;
    arr[static_cast<size_t>(Key::Q)] = GLFW_KEY_Q;
    arr[static_cast<size_t>(Key::R)] = GLFW_KEY_R;
    arr[static_cast<size_t>(Key::S)] = GLFW_KEY_S;
    arr[static_cast<size_t>(Key::T)] = GLFW_KEY_T;
    arr[static_cast<size_t>(Key::U)] = GLFW_KEY_U;
    arr[static_cast<size_t>(Key::V)] = GLFW_KEY_V;
    arr[static_cast<size_t>(Key::W)] = GLFW_KEY_W;
    arr[static_cast<size_t>(Key::X)] = GLFW_KEY_X;
    arr[static_cast<size_t>(Key::Y)] = GLFW_KEY_Y;
    arr[static_cast<size_t>(Key::Z)] = GLFW_KEY_Z;
    arr[static_cast<size_t>(Key::Space)] = GLFW_KEY_SPACE;
    arr[static_cast<size_t>(Key::LeftControl)] = GLFW_KEY_LEFT_CONTROL;
    arr[static_cast<size_t>(Key::LeftAlt)] = GLFW_KEY_LEFT_ALT;
    arr[static_cast<size_t>(Key::Escape)] = GLFW_KEY_ESCAPE;
    arr[static_cast<size_t>(Key::F1)] = GLFW_KEY_F1;

    return arr;
  }();

  const auto index = static_cast<size_t>(k);
  if (index >= map.size())
    return GLFW_KEY_UNKNOWN;
  DEBUG_ASSERT(map[index] != GLFW_KEY_UNKNOWN,
               std::format("Mapping should exist for key (int): {}",
                           static_cast<size_t>(k)));
  return map[index];
}

[[nodiscard]] constexpr KeyState getState(KeyState last, bool isDown) noexcept {

  if (isDown) {
    if (last == KeyState::Idle || last == KeyState::Released) {
      return KeyState::Pressed;
    }
    if (last == KeyState::Pressed || last == KeyState::Held) {
      return KeyState::Held;
    }
  } else {
    if (last == KeyState::Idle || last == KeyState::Released) {
      return KeyState::Idle;
    }
    if (last == KeyState::Pressed || last == KeyState::Held) {
      return KeyState::Released;
    }
  }

  DEBUG_ASSERT(false, "All paths should be covered");
  std::unreachable();
}

enum class MouseButton : std::uint8_t { Left, Right, __SizeGuard };

[[nodiscard]] constexpr GLFWMouseButton
mouseButtonToGLFW(MouseButton button) noexcept {

  static constexpr auto map = []() {
    std::array<GLFWMouseButton, static_cast<size_t>(MouseButton::__SizeGuard)>
        arr{};
    arr.fill(GLFW_KEY_UNKNOWN);
    arr[static_cast<size_t>(MouseButton::Left)] = GLFW_MOUSE_BUTTON_LEFT;
    arr[static_cast<size_t>(MouseButton::Right)] = GLFW_MOUSE_BUTTON_RIGHT;
    return arr;
  }();

  const auto index = static_cast<size_t>(button);
  if (index >= map.size())
    return GLFW_KEY_UNKNOWN;
  DEBUG_ASSERT(map[index] != GLFW_KEY_UNKNOWN, "Mapping should exist");
  return map[index];
}

struct Input {
  constexpr Input() noexcept : keyStates{}, mouseStates{} {}

  Input(const Input &other) = delete;
  Input(Input &&other) = delete;
  Input &operator=(Input &&other) = delete;
  Input &operator=(const Input &other) = delete;

  constexpr void update(GLFWwindow *window) noexcept {

    // Updating keyboard
    constexpr size_t keyCount = static_cast<size_t>(Key::__SizeGuard);
    for (size_t i = 0; i < keyCount; ++i) {
      Key k = static_cast<Key>(i);
      GLFWKeyCode GLFWcode = keyToGLFW(k);

      if (GLFWcode != GLFW_KEY_UNKNOWN) {
        bool isDown = glfwGetKey(window, GLFWcode) == GLFW_PRESS;

        DEBUG_ASSERT(glfwGetKey(window, GLFWcode) == GLFW_PRESS ||
                         glfwGetKey(window, GLFWcode) == GLFW_RELEASE,
                     "glfwGetKey 'should' have only two states");

        keyStates[i] = getState(keyStates[i], isDown);
      }
    }

    // Updating mouse click states
    constexpr size_t mouseButtons =
        static_cast<size_t>(MouseButton::__SizeGuard);
    for (size_t i = 0; i < mouseButtons; ++i) {
      MouseButton b = static_cast<MouseButton>(i);
      GLFWMouseButton GLFWcode = mouseButtonToGLFW(b);

      if (GLFWcode != GLFW_KEY_UNKNOWN) {
        bool isDown = glfwGetMouseButton(window, GLFWcode) == GLFW_PRESS;

        DEBUG_ASSERT(glfwGetMouseButton(window, GLFWcode) == GLFW_PRESS ||
                         glfwGetMouseButton(window, GLFWcode) == GLFW_RELEASE,
                     "glfwGetKey 'should' have only two states");

        mouseStates[i] = getState(mouseStates[i], isDown);
      }
    }

    // Updating mouse position
    lastMousePosition = mousePosition;
    glfwGetCursorPos(window, &(mousePosition.data[0][0]),
                     &(mousePosition.data[0][1]));
  }

  [[nodiscard]] constexpr bool isPressed(Key k) const noexcept {
    return keyStates[static_cast<size_t>(k)] == KeyState::Pressed;
  }

  [[nodiscard]] constexpr bool isReleased(Key k) const noexcept {
    return keyStates[static_cast<size_t>(k)] == KeyState::Released;
  }

  [[nodiscard]] constexpr bool isHeld(Key k) const noexcept {
    return keyStates[static_cast<size_t>(k)] == KeyState::Held;
  }

  [[nodiscard]] constexpr bool isUp(Key k) const noexcept {
    return keyStates[static_cast<size_t>(k)] == KeyState::Idle ||
           keyStates[static_cast<size_t>(k)] == KeyState::Released;
  }

  [[nodiscard]] constexpr bool isDown(Key k) const noexcept {
    return keyStates[static_cast<size_t>(k)] == KeyState::Pressed ||
           keyStates[static_cast<size_t>(k)] == KeyState::Held;
  }

  [[nodiscard]] constexpr bool isPressed(MouseButton k) const noexcept {
    return mouseStates[static_cast<size_t>(k)] == KeyState::Pressed;
  }

  [[nodiscard]] constexpr bool isReleased(MouseButton k) const noexcept {
    return mouseStates[static_cast<size_t>(k)] == KeyState::Released;
  }

  [[nodiscard]] constexpr bool isHeld(MouseButton k) const noexcept {
    return mouseStates[static_cast<size_t>(k)] == KeyState::Held;
  }

  [[nodiscard]] constexpr bool isUp(MouseButton k) const noexcept {
    return mouseStates[static_cast<size_t>(k)] == KeyState::Idle ||
           mouseStates[static_cast<size_t>(k)] == KeyState::Released;
  }

  [[nodiscard]] constexpr bool isDown(MouseButton k) const noexcept {
    return mouseStates[static_cast<size_t>(k)] == KeyState::Pressed ||
           mouseStates[static_cast<size_t>(k)] == KeyState::Held;
  }

  [[nodiscard]] constexpr v2d getMouseDelta() const noexcept {
    return mousePosition - lastMousePosition;
  }

private:
  // Keyboard
  std::array<KeyState, static_cast<size_t>(Key::__SizeGuard)> keyStates{};

  // Mouse
  std::array<KeyState, static_cast<size_t>(MouseButton::__SizeGuard)>
      mouseStates{};

  v2d lastMousePosition{};
  v2d mousePosition{};
};
