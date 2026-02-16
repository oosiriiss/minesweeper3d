#include "game/board.hpp"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <logzy/logzy.hpp>
#include <random>

#include "debug_utils.hpp"
#include "math/matrix.hpp"
#include "render/camera.hpp"

static void errorCallback(int code, const char *description) {
  logzy::error("GLFW Error occurred. Code {}. Description: {}", code,
               description);
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {

  DEBUG_ONLY(std::println("HELLO!"));

  // Initializing glfw

  if (!glfwInit()) {
    logzy::critical("GLFW not intialized\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 800, "My window", nullptr, nullptr);

  if (!window) {
    logzy::critical("Couldn't create GLFW window\n");
    return -2;
  }

  glfwSetErrorCallback(errorCallback);
  glfwSetKeyCallback(window, keyCallback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Creating openGL context
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);

  Board board;

  constexpr std::uint32_t BOARD_SIZE{10};

  if (auto boardOpt = Board::create(v3u{BOARD_SIZE, BOARD_SIZE, BOARD_SIZE})) {
    board = std::move(*boardOpt);
  } else {
    logzy::critical("Couldn't create board");
    return -1;
  }

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type>
      boardIndexDistribution(0, BOARD_SIZE - 1);

  constexpr v3f cameraInitialPosition = vec3<float>(.0F, .0F, 20.0F);
  constexpr v3f cameraArbitraryUp = vec3<float>(0.0F, 1.0F, 0.0F);
  constexpr float cameraSpeed = 10.0F;
  constexpr float horizontalSensitivity = 0.8f;
  constexpr float verticalSensitivity = horizontalSensitivity / 1.5f;

  Camera camera(cameraInitialPosition, cameraArbitraryUp);

  v2d mousePos;
  glfwGetCursorPos(window, &(mousePos.data[0][0]), &(mousePos.data[0][1]));

  double lastTime = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double time = static_cast<float>(glfwGetTime());

    // Handling mouse movement
    v2d newMousePos;
    glfwGetCursorPos(window, &(newMousePos.data[0][0]),
                     &(newMousePos.data[0][1]));
    v2d mouseDelta = newMousePos - mousePos;

    // TODO :: Investigate why does yaw have to be negated in order to rotate
    // in the right direction
    camera.rotate(vec3<float>(
        static_cast<float>(mouseDelta.data[0][1]) * horizontalSensitivity,
        static_cast<float>(-mouseDelta.data[0][0]) * verticalSensitivity,
        0.0F));

    mousePos = newMousePos;

    float dt = lastTime - time;
    lastTime = time;

    float cameraDistance = cameraSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_A)) {
      camera.move(Camera::Direction::Left, cameraDistance);
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
      camera.move(Camera::Direction::Right, cameraDistance);
    }
    if (glfwGetKey(window, GLFW_KEY_W)) {
      camera.move(Camera::Direction::Forward, cameraDistance);
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
      camera.move(Camera::Direction::Backward, cameraDistance);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
      camera.move(Camera::Direction::Up, cameraDistance);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
      camera.move(Camera::Direction::Down, cameraDistance);
    }
    if (glfwGetKey(window, GLFW_KEY_B)) {
      board.changeCubeSize(0.01);
      logzy::debug("Cell size is now: {}", board.cellSize);
    }
    if (glfwGetKey(window, GLFW_KEY_N)) {
      board.changeCubeSize(-0.01);
      logzy::debug("Cell size is now: {}", board.cellSize);
    }
    if (glfwGetKey(window, GLFW_KEY_C)) {
      board.testCollisions(camera.position, camera.direction);
    }

    if (glfwGetKey(window, GLFW_KEY_P)) {
      logzy::debug("Camera position: {}\nCamera direction: {}\nCell[0][0][0] "
                   "position: {}",
                   camera.position, camera.direction,
                   board.cellPosition(vec3<size_t>(0, 0, 0)));
    }

    // if (glfwGetKey(window, GLFW_KEY_P)) {
    //   v3u coords{static_cast<unsigned int>(boardIndexDistribution(rng)),
    //              static_cast<unsigned int>(boardIndexDistribution(rng)),
    //              static_cast<unsigned int>(boardIndexDistribution(rng))};

    //  board.dig(coords);
    //}

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float ratio = static_cast<float>(width) / height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const m4x4f &v = camera.getView();

    constexpr float fov = 50.0f;
    constexpr float near = 0.01f;
    constexpr float far = 100.0f;
    auto p = perspective(fov, ratio, near, far);

    board.draw(v, p);

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  // Shutting odwn glfw
  glfwTerminate();

  return 0;
}
