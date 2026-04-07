#pragma once
#include "input.hpp"

struct Application {

  auto initialize() -> bool;
  void run();
  auto shutdown() -> bool;

private:
  // Whether user is playing the game of moving cursor around (i.e for ui)
  Input input_{};
  GLFWwindow *mainWindow_{nullptr};
};
