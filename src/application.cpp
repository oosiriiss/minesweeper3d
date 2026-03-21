#include "application.hpp"

#include <GLFW/glfw3.h>
#include <logzy/logzy.hpp>

#include "debug_utils.hpp"
#include "game/crosshair.hpp"
#include "glad.h"
#include "render/camera.hpp"
#include "resource_manager.hpp"

static void GLFWErrorCallback(int code, const char *description) {
  logzy::error("GLFW Error occurred. Code {}. Description: {}", code,
               description);
}

static void GLFWKeyCallback(GLFWwindow *window, int key, int scancode,
                            int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void loadTextures() {
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

static auto initializeGLFW() -> bool {
  if (!glfwInit()) {
    logzy::critical("GLFW could not be initialized. glfwInit() failed.");
    return false;
  }
  return true;
}

/**
 * Creates  GLFW window with OPENGL 4.6 core as render context.
 */
static auto createGLFWWindow() -> GLFWwindow * {
  // Why not use the newest one I guess
  constexpr int OPENGL_VERSION_MAJOR = 4;
  constexpr int OPENGL_VERSION_MINOR = 6;

  // Not using deprecated functions, no compatiblilty with older opengl
  constexpr int OPENGL_PROFILE = GLFW_OPENGL_CORE_PROFILE;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, OPENGL_PROFILE);

  constexpr std::uint32_t INITIAL_WINDOW_WIDTH = 800;
  constexpr std::uint32_t INITIAL_WINDOW_HEIGHT = 800;
  constexpr const char *MAIN_WINDOW_TITLE = "Minesweeper 3D";

  GLFWwindow *window =
      glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
                       MAIN_WINDOW_TITLE, nullptr, nullptr);

  return window;
}

static auto initializeMainGLFWWindow(GLFWwindow *window) -> bool {
  // Callbacks
  DEBUG_ASSERT(glfwSetErrorCallback(nullptr) == nullptr,
               "Making sure no duplicate error callback is set");
  DEBUG_ASSERT(glfwSetKeyCallback(window, nullptr) == nullptr,
               "Making sure no duplicate key callback is set");

  // TODO :: In theory these could fail too
  glfwSetErrorCallback(GLFWErrorCallback);
  glfwSetKeyCallback(window, GLFWKeyCallback);

  // Disabling cursor when focused
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  return true;
}

static void intializeOpenGL(GLFWwindow *window) {
  // TODO :: In theory these could fail too

  // OpenGL stuff
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);

  // VSYNC ON
  glfwSwapInterval(1);

  // OpenGl global state
  glEnable(GL_DEPTH_TEST);
  // for transparnets
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

auto Application::initialize() -> bool {
  if (!initializeGLFW()) {
    logzy::critical("GLFW could not be initialized");
    return false;
  }

  mainWindow_ = createGLFWWindow();

  if (mainWindow_ == nullptr) {
    logzy::critical("Couldn't create the main window of the application.");
    return false;
  }

  if (!initializeMainGLFWWindow(mainWindow_)) {
    logzy::critical("Initialization of main window failed.");
    return false;
  }

  intializeOpenGL(mainWindow_);

  // TODO :: Later scenes should load assets they need
  loadTextures();

  return true;
}

static void handleInputs(const Input &input, Board &board, Camera &camera,
                         float dt) {

  constexpr float cameraSpeed = 10.0F;
  constexpr float horizontalSensitivity = 5.f;
  constexpr float verticalSensitivity = horizontalSensitivity / 1.5f;

  // Mouse movement
  v2d mouseDelta = input.getMouseDelta();
  camera.rotate(vec3<float>(
      static_cast<float>(mouseDelta.data[0][1]) * horizontalSensitivity * dt,
      static_cast<float>(-mouseDelta.data[0][0]) * verticalSensitivity * dt,
      0.0F));

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
}

static void drawHUD(const Crosshair &cs, const m4x4f &proj) {
  // Drawing ui
  // Static ui doesnt need depth
  glDisable(GL_DEPTH_TEST);
  // Crosshair
  cs.draw(proj);

  // Resetting depth test
  glEnable(GL_DEPTH_TEST);
}

void Application::run() {

  // Configs
  constexpr v3f cameraInitialPosition = vec3<float>(.0F, .0F, 20.0F);
  constexpr v3f cameraArbitraryUp = vec3<float>(0.0F, 1.0F, 0.0F);

  // TODO :: Not paying too much attention to this as it will be refactored into
  // scenes later
  Board board;
  constexpr size_t BOARD_SIZE{10};
  if (auto boardOpt = Board::create(v3uz{BOARD_SIZE, BOARD_SIZE, BOARD_SIZE})) {
    board = std::move(*boardOpt);
  } else {
    logzy::critical("Couldn't create board");
    return;
  }

  Camera camera(cameraInitialPosition, cameraArbitraryUp);
  v2d mousePos;
  glfwGetCursorPos(mainWindow_, &(mousePos.data[0][0]), &(mousePos.data[0][1]));
  double lastTime = glfwGetTime();

  // HUD
  int windowWidth = -1;
  int windowHeight = -1;
  glfwGetWindowSize(mainWindow_, &windowWidth, &windowHeight);
  Crosshair crosshair(vec2<std::uint32_t>(windowWidth, windowHeight),
                      vec2(10u, 10u), vec3(0.0f, 0.0f, 1.0f));

  constexpr float fov = 50.0f;
  constexpr float near = 0.01f;
  constexpr float far = 100.0f;
  const float ratio = static_cast<float>(windowWidth) / windowHeight;
  auto persp = perspective(fov, ratio, near, far);
  auto ortho = orthographic(0.0f, windowWidth, 0.0F, windowHeight, -1.0f);

  while (!glfwWindowShouldClose(mainWindow_)) {
    glfwPollEvents();
    input_.update(mainWindow_);

    double time = static_cast<float>(glfwGetTime());
    float dt = time - lastTime;
    lastTime = time;
    handleInputs(input_, board, camera, dt);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const m4x4f &v = camera.getView();

    board.draw(v, persp);
    drawHUD(crosshair, ortho);

    glfwSwapBuffers(mainWindow_);
  }
}

auto Application::shutdown() -> bool {
  if (mainWindow_ == nullptr) {
    logzy::error("Trying to shutdown application that wasn't initialized with "
                 "Application::initialize()");
    return false;
  }
  glfwDestroyWindow(mainWindow_);

  // Shutting odwn glfw
  glfwTerminate();
  return true;
}
