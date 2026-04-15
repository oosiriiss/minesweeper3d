#include "input.hpp"
#include "tasty/tasty.hpp"

int main() {
  bool allPassed = true;

  {
    tasty::TestRunner suite("getState transitions");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Pressed, getState(KeyState::Idle, true));
        },
        "Idle + down -> Pressed");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Pressed,
                             getState(KeyState::Released, true));
        },
        "Released + down -> Pressed");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Held, getState(KeyState::Pressed, true));
        },
        "Pressed + down -> Held");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Held, getState(KeyState::Held, true));
        },
        "Held + down -> Held");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Idle, getState(KeyState::Idle, false));
        },
        "Idle + up -> Idle");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Idle,
                             getState(KeyState::Released, false));
        },
        "Released + up -> Idle");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Released,
                             getState(KeyState::Pressed, false));
        },
        "Pressed + up -> Released");

    suite.registerTest(
        []() {
          tasty::expectEqual(KeyState::Released,
                             getState(KeyState::Held, false));
        },
        "Held + up -> Released");

    allPassed &= suite.runAll();
  }

  {
    tasty::TestRunner suite("keyToGLFW");

    suite.registerTest([]() { TASTY_EXPECT(keyToGLFW(Key::A) == GLFW_KEY_A); },
                       "Key::A maps to GLFW_KEY_A");

    suite.registerTest([]() { TASTY_EXPECT(keyToGLFW(Key::Z) == GLFW_KEY_Z); },
                       "Key::Z maps to GLFW_KEY_Z");

    suite.registerTest(
        []() { TASTY_EXPECT(keyToGLFW(Key::Space) == GLFW_KEY_SPACE); },
        "Key::Space maps to GLFW_KEY_SPACE");

    suite.registerTest(
        []() { TASTY_EXPECT(keyToGLFW(Key::Escape) == GLFW_KEY_ESCAPE); },
        "Key::Escape maps to GLFW_KEY_ESCAPE");

    suite.registerTest(
        []() { TASTY_EXPECT(keyToGLFW(Key::F1) == GLFW_KEY_F1); },
        "Key::F1 maps to GLFW_KEY_F1");

    suite.registerTest(
        []() {
          TASTY_EXPECT(keyToGLFW(Key::LeftControl) == GLFW_KEY_LEFT_CONTROL);
        },
        "Key::LeftControl maps to GLFW_KEY_LEFT_CONTROL");

    suite.registerTest(
        []() { TASTY_EXPECT(keyToGLFW(Key::LeftAlt) == GLFW_KEY_LEFT_ALT); },
        "Key::LeftAlt maps to GLFW_KEY_LEFT_ALT");

    // Key::Unknown is past __SizeGuard -> should return GLFW_KEY_UNKNOWN
    suite.registerTest(
        []() { TASTY_EXPECT(keyToGLFW(Key::Unknown) == GLFW_KEY_UNKNOWN); },
        "Key::Unknown returns GLFW_KEY_UNKNOWN");

    allPassed &= suite.runAll();
  }

  {
    tasty::TestRunner suite("mouseButtonToGLFW");

    suite.registerTest(
        []() {
          TASTY_EXPECT(mouseButtonToGLFW(MouseButton::Left) ==
                       GLFW_MOUSE_BUTTON_LEFT);
        },
        "MouseButton::Left maps to GLFW_MOUSE_BUTTON_LEFT");

    suite.registerTest(
        []() {
          TASTY_EXPECT(mouseButtonToGLFW(MouseButton::Right) ==
                       GLFW_MOUSE_BUTTON_RIGHT);
        },
        "MouseButton::Right maps to GLFW_MOUSE_BUTTON_RIGHT");

    allPassed &= suite.runAll();
  }

  {
    tasty::TestRunner suite("Input key query helpers (state logic)");

    suite.registerTest(
        []() {
          // After Idle->down the state is Pressed
          const KeyState s = getState(KeyState::Idle, true);
          TASTY_EXPECT(s == KeyState::Pressed);
          // isDown must also be true for Pressed
          TASTY_EXPECT(s == KeyState::Pressed || s == KeyState::Held);
        },
        "Pressed state satisfies isPressed and isDown semantics");

    suite.registerTest(
        []() {
          const KeyState s = getState(KeyState::Pressed, true);
          TASTY_EXPECT(s == KeyState::Held);
          // isDown must also be true for Held
          TASTY_EXPECT(s == KeyState::Pressed || s == KeyState::Held);
        },
        "Held state satisfies isHeld and isDown semantics");

    suite.registerTest(
        []() {
          const KeyState s = getState(KeyState::Held, false);
          TASTY_EXPECT(s == KeyState::Released);
          // isUp must also be true for Released
          TASTY_EXPECT(s == KeyState::Idle || s == KeyState::Released);
        },
        "Released state satisfies isReleased and isUp semantics");

    suite.registerTest(
        []() {
          const KeyState s = getState(KeyState::Released, false);
          TASTY_EXPECT(s == KeyState::Idle);
          TASTY_EXPECT(s == KeyState::Idle || s == KeyState::Released);
        },
        "Idle state satisfies isUp semantics");

    suite.registerTest(
        []() {
          const KeyState s = KeyState::Idle;
          const bool isDown = (s == KeyState::Pressed || s == KeyState::Held);
          TASTY_EXPECT(!isDown);
        },
        "Idle state does NOT satisfy isDown");

    suite.registerTest(
        []() {
          const KeyState s = KeyState::Pressed;
          const bool isUp = (s == KeyState::Idle || s == KeyState::Released);
          TASTY_EXPECT(!isUp);
        },
        "Pressed state does NOT satisfy isUp");

    allPassed &= suite.runAll();
  }

  {
    tasty::TestRunner suite("getState full round-trip sequence");

    suite.registerTest(
        []() {
          KeyState s = KeyState::Idle;

          s = getState(s, true);
          tasty::expectEqual(KeyState::Pressed, s,
                             std::source_location::current());

          s = getState(s, true);
          tasty::expectEqual(KeyState::Held, s,
                             std::source_location::current());

          s = getState(s, true);
          tasty::expectEqual(KeyState::Held, s,
                             std::source_location::current());

          s = getState(s, false);
          tasty::expectEqual(KeyState::Released, s,
                             std::source_location::current());

          s = getState(s, false);
          tasty::expectEqual(KeyState::Idle, s,
                             std::source_location::current());
        },
        "Idle -> Pressed -> Held -> Held -> Released -> Idle");

    allPassed &= suite.runAll();
  }

  return allPassed ? 0 : 1;
}
