#pragma once

#include "GLFW/glfw3.h"
struct ProfilerData {
  double totalFrameMs{0.0};
  double updateMs{0.0};
  double cpuRenderMs{0.0};
  double gpuRenderMs{0.0};
  double uiMs{0.0};
  double waitTime{0.0};
  std::uint64_t frameCounter{0};
};

struct ScopedTimer {
  double start;
  double &outputMs;
  ScopedTimer(double &out) : outputMs{out} { start = glfwGetTime(); }
  ~ScopedTimer() {
    // Second to ms
    outputMs = (glfwGetTime() - start) * 1000.0;
  }
};
