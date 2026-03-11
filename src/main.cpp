#include "game/board.hpp"
#include "game/crosshair.hpp"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <logzy/logzy.hpp>
#include <random>
#include <thread>

#include "debug_utils.hpp"
#include "game/resource_manager.hpp"
#include "input.hpp"
#include "math/math.hpp"
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

void loadTextures() {

  std::array<std::string_view, 28> paths{
      "tiles/tile_0.jpg",   "tiles/tile_1.jpg",  "tiles/tile_2.jpg",
      "tiles/tile_3.jpg",   "tiles/tile_4.jpg",  "tiles/tile_5.jpg",
      "tiles/tile_6.jpg",   "tiles/tile_7.jpg",  "tiles/tile_8.jpg",
      "tiles/tile_9.jpg",   "tiles/tile_10.jpg", "tiles/tile_11.jpg",
      "tiles/tile_12.jpg",  "tiles/tile_13.jpg", "tiles/tile_14.jpg",
      "tiles/tile_15.jpg",  "tiles/tile_16.jpg", "tiles/tile_17.jpg",
      "tiles/tile_18.jpg",  "tiles/tile_19.jpg", "tiles/tile_20.jpg",
      "tiles/tile_21.jpg",  "tiles/tile_22.jpg", "tiles/tile_23.jpg",
      "tiles/tile_24.jpg",  "tiles/tile_25.jpg", "tiles/tile_26.jpg",
      "tiles/tile_flag.jpg"};

  if (ResourceManager::loadTextureArray(ResourceManager::TileTexturesKey,
                                        std::span{paths})) {
    logzy::info("Loaded texture array: {}", ResourceManager::TileTexturesKey);
  } else {
    logzy::critical("Failed to load texture: {}",
                    ResourceManager::TileTexturesKey);
  }
}

int main() {

  // TODO :: resizing

  DEBUG_ONLY(std::println("HELLO!"));

  // Initializing glfw

  if (!glfwInit()) {
    logzy::critical("GLFW not intialized\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  constexpr std::uint32_t SCREEN_WIDTH = 800;
  constexpr std::uint32_t SCREEN_HEIGHT = 800;

  GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                                        "My window", nullptr, nullptr);

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

  // Enable color alpha channel blending
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(0.0, 0.0, 0.0, 0.0);

  loadTextures();
  Board board;

  constexpr size_t BOARD_SIZE{10};

  if (auto boardOpt = Board::create(v3uz{BOARD_SIZE, BOARD_SIZE, BOARD_SIZE})) {
    board = std::move(*boardOpt);
  } else {
    logzy::critical("Couldn't create board");
    return -1;
  }

  Input input(window);

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type>
      boardIndexDistribution(0, BOARD_SIZE - 1);

  constexpr v3f cameraInitialPosition = vec3<float>(.0F, .0F, 20.0F);
  constexpr v3f cameraArbitraryUp = vec3<float>(0.0F, 1.0F, 0.0F);
  constexpr float cameraSpeed = 10.0F;
  constexpr float horizontalSensitivity = 0.5f;
  constexpr float verticalSensitivity = horizontalSensitivity / 1.5f;

  Camera camera(cameraInitialPosition, cameraArbitraryUp);

  v2d mousePos;
  glfwGetCursorPos(window, &(mousePos.data[0][0]), &(mousePos.data[0][1]));

  double lastTime = glfwGetTime();

  // HUD
  Crosshair crosshair(vec2(SCREEN_WIDTH, SCREEN_HEIGHT), vec2(10u, 10u),
                      vec3(0.0f, 0.0f, 1.0f));

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    input.update(window);

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

    if (input.isDown(Key::A)) {
      camera.move(Camera::Direction::Left, cameraDistance);
    }
    if (input.isDown(Key::D)) {
      camera.move(Camera::Direction::Right, cameraDistance);
    }
    if (input.isDown(Key::W)) {
      camera.move(Camera::Direction::Forward, cameraDistance);
    }
    if (input.isDown(Key::S)) {
      camera.move(Camera::Direction::Backward, cameraDistance);
    }
    if (input.isDown(Key::Space)) {
      camera.move(Camera::Direction::Up, cameraDistance);
    }
    if (input.isDown(Key::LeftControl)) {
      camera.move(Camera::Direction::Down, cameraDistance);
    }

    if (input.isPressed(Key::B)) {
      board.changeCubeSize(0.01);
      logzy::debug("Cell size is now: {}", board.cellSize);
    }

    if (input.isPressed(Key::N)) {
      board.changeCubeSize(-0.01);
      logzy::debug("Cell size is now: {}", board.cellSize);
    }

    if (input.isPressed(Key::N)) {
      board.toggleDrawNeighbours(!board.drawDugAdjacent);
    }

    if (input.isPressed(MouseButton::Left)) {
      board.onLeftClick(camera.position, camera.getDirection());
    }

    if (input.isPressed(MouseButton::Right)) {
      board.onRightClick(camera.position, camera.getDirection());
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float ratio = static_cast<float>(width) / height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const m4x4f &v = camera.getView();

    constexpr float fov = 50.0f;
    constexpr float near = 0.01f;
    constexpr float far = 100.0f;
    auto persp = perspective(fov, ratio, near, far);

    board.draw(v, persp);

    // Drawing ui
    // Static ui doesnt need depth
    glDisable(GL_DEPTH_TEST);
    auto ortho = orthographic(0.0f, SCREEN_WIDTH, 0.0F, SCREEN_HEIGHT, -1.0f);

    // Crosshair
    crosshair.draw(ortho);

    // Resetting depth test
    glEnable(GL_DEPTH_TEST);

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  // Shutting odwn glfw
  glfwTerminate();

  return 0;
}
