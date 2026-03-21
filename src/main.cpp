#include "application.hpp"

int main() {
  Application app;

  if (!app.initialize()) {
    return -1;
  }
  app.run();
  if (!app.shutdown()) {
    return -2;
  }

  return 0;
}
