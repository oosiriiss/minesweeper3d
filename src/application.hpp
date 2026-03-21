#pragma once
#include "input.hpp"

struct Application {

  auto initialize() -> bool;
  void run();
  auto shutdown() -> bool;

private:
  Input input_{};
  GLFWwindow *mainWindow_{nullptr};
};
