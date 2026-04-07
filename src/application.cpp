#include "application.hpp"

#include <GLFW/glfw3.h>
#include <logzy/logzy.hpp>
#include <set>
#include <system_error>

#include "debug_utils.hpp"
#include "game/board.hpp"
#include "game/crosshair.hpp"
#include "glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "profiling.hpp"
#include "render/camera.hpp"
#include "resource_manager.hpp"
#include "settings.hpp"

static void GLFWErrorCallback(int code, const char *description) {
  logzy::error("GLFW Error occurred. Code {}. Description: {}", code,
               description);
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
  glEnable(GL_BLEND);
  // for transparnets
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

static void initializeDearImgui(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  float scale =
      ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(scale);
  style.FontScaleDpi = scale;

  bool installCallbacks = true;
  ImGui_ImplGlfw_InitForOpenGL(window, installCallbacks);
  ImGui_ImplOpenGL3_Init("#version 330 core");
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
  initializeDearImgui(mainWindow_);

  // TODO :: Later scenes should load assets they need
  loadTextures();

  return true;
}

static void handleInputs(const Input &input, const Settings &settings,
                         Board &board, Camera &camera, GLFWwindow *window,
                         bool &menuOpen, float dt) {

  // Showing cursor when alt is pressed
  if (input.isPressed(Key::Escape)) {
    menuOpen = !menuOpen;
    if (menuOpen) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }

  // Game inputs are ignored in cursor/ui mode
  if (menuOpen) {
    return;
  }

  //////////////////////////////////
  /// Game inputs

  // Mouse movement
  v2d mouseDelta = input.getMouseDelta();
  camera.rotate(vec3<float>(
      static_cast<float>(mouseDelta.data[0][1]) * settings.sensitivity * dt,
      static_cast<float>(-mouseDelta.data[0][0]) * settings.sensitivity * dt,
      0.0F));

  float cameraDistance = settings.cameraSpeed * dt;

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

static void drawMenu(GLFWwindow *window, Settings &settings) {

  constexpr float minMovementSpeed = 1.0f;
  constexpr float maxMovementSpeed = 20.0f;

  constexpr float minSensitivity = 0.01f;
  constexpr float maxSensitivity = 20.0f;

  {
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoMove;
    // flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    // flags |= ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("Settings", nullptr, flags);

    ImGui::SetWindowFontScale(3.0f);
    ImVec2 cursorBeforeMenu = ImGui::GetCursorPos();

    const char *menuText = "Menu";
    ImVec2 menuTextSize = ImGui::CalcTextSize(menuText);
    float menuX = (vp->WorkSize.x - menuTextSize.x) * 0.5f;
    ImGui::SetCursorPos((ImVec2(menuX, cursorBeforeMenu.y)));
    ImGui::Text("%s", menuText);
    ImGui::SetCursorPos(
        (ImVec2(cursorBeforeMenu.x, cursorBeforeMenu.y + menuTextSize.y)));
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Separator();

    {
      ImGui::Text("%s", "Settings");
      constexpr float sliderMaxWidth = 400.0f;
      float availableWidth = ImGui::GetContentRegionAvail().x;
      float sliderWidth = std::min(availableWidth, sliderMaxWidth);
      ImGui::PushItemWidth(sliderWidth);
      ImGui::SliderFloat("Mouse sensitivity", &settings.sensitivity,
                         minSensitivity, maxSensitivity);
      ImGui::SliderFloat("Player movement speed", &settings.cameraSpeed,
                         minMovementSpeed, maxMovementSpeed);
      ImGui::PopItemWidth();
    }
    if (ImGui::Button("Exit")) {
      glfwSetWindowShouldClose(window, true);
    }
    ImGui::End();
  }
}

void drawRenderData(const ProfilerData &data) {

  ImGui::SetNextWindowPos({0, 0});
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  flags |= ImGuiWindowFlags_NoNavFocus;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  ImGui::Begin("Frame data", nullptr, flags);

  ImGui::Text("FPS: %d",
              static_cast<std::uint32_t>(1.0 / data.totalFrameMs * 1000.0));
  ImGui::Text("Frame time [ms]: %.3f", data.totalFrameMs);
  ImGui::Text("CPU update time [ms]: %.3f", data.updateMs);
  ImGui::Text("CPU Render time [ms]: %.3f", data.cpuRenderMs);
  ImGui::Text("GPU Render time [ms]: %.3f", data.gpuRenderMs);
  ImGui::Text("UI Update time [ms]: %.3f", data.uiUpdateMs);
  ImGui::Text("UI Render time [ms]: %.3f", data.uiRenderMs);
  ImGui::Text("Wait  time [ms]: %.3f", data.waitTime);
  ImGui::Text("Frame number: %llu", data.frameCounter);

  ImGui::End();
}

static void drawHUD(const Crosshair &cs, const m4x4f &proj) {
  // Drawing ui
  // Static ui doesnt need depth
  // glDisable(GL_DEPTH_TEST);
  //

  // Crosshair
  cs.draw(proj);

  // Resetting depth test
  // glEnable(GL_DEPTH_TEST);
}

void Application::run() {

  // Configs
  constexpr v3f cameraInitialPosition = vec3<float>(.0F, .0F, 20.0F);
  constexpr v3f cameraArbitraryUp = vec3<float>(0.0F, 1.0F, 0.0F);

  bool menuOpen{false};
  Settings settings;

  // TODO :: Not paying too much attention to this as it will be refactored
  // into scenes later
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

  // Profilers

  // triple buffering
  constexpr int queryBuffers = 3;
  GLuint queryID[queryBuffers];
  glGenQueries(queryBuffers, queryID);

  printf("queryID[0]=%u queryID[1]=%u\n", queryID[0], queryID[1]);

  ProfilerData profilerData{};

  static GLsync frameSync = nullptr;

  while (!glfwWindowShouldClose(mainWindow_)) {
    glfwPollEvents();
    input_.update(mainWindow_);

    double time = static_cast<float>(glfwGetTime());
    float dt = time - lastTime;
    lastTime = time;
    ++profilerData.frameCounter;
    {
      ScopedTimer waitTimer(profilerData.waitTime);
      if (frameSync) {
        // Czekamy aż GPU faktycznie skończy poprzednią klatkę
        glClientWaitSync(frameSync, GL_SYNC_FLUSH_COMMANDS_BIT,
                         1000000000); // timeout 1s
        glDeleteSync(frameSync);
        frameSync = nullptr;
      }
    }

    // Seconds to ms
    profilerData.totalFrameMs = dt * 1000.0;
    {
      ScopedTimer updateTimer(profilerData.updateMs);
      handleInputs(input_, settings, board, camera, mainWindow_, menuOpen, dt);
    }

    {
      ScopedTimer renderTimer(profilerData.cpuRenderMs);
      // Writing
      const int frontBuffer = profilerData.frameCounter % queryBuffers;
      // Reading buffer delayed by queryBuffers-1 frames
      const int backBuffer =
          (profilerData.frameCounter - (queryBuffers - 1) + queryBuffers) %
          queryBuffers;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      const m4x4f &v = camera.getView();

      glBeginQuery(GL_TIME_ELAPSED, queryID[frontBuffer]);

      board.draw(v, persp);
      drawHUD(crosshair, ortho);

      glEndQuery(GL_TIME_ELAPSED);

      GLuint available = 1;
      glGetQueryObjectuiv(queryID[backBuffer], GL_QUERY_RESULT_AVAILABLE,
                          &available);

      if (profilerData.frameCounter >= 3 && available) {
        GLuint64 nanosElapsed = 0;
        glGetQueryObjectui64v(queryID[backBuffer], GL_QUERY_RESULT,
                              &nanosElapsed);
        profilerData.gpuRenderMs = nanosElapsed / 1'000'000.0;
      }
    }
    {
      ScopedTimer uiTimer(profilerData.uiUpdateMs);

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      drawRenderData(profilerData);

      if (menuOpen) {
        drawMenu(mainWindow_, settings);
      }

      ImGui::Render();
    }
    {
      ScopedTimer uiTimer(profilerData.uiRenderMs);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    {
      ScopedTimer systemTimer(profilerData.waitTime);
      glfwSwapBuffers(mainWindow_);
      frameSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
  }
  glDeleteQueries(2, queryID);
}

auto Application::shutdown() -> bool {
  if (mainWindow_ == nullptr) {
    logzy::error("Trying to shutdown application that wasn't initialized with "
                 "Application::initialize()");
    return false;
  }
  glfwDestroyWindow(mainWindow_);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // Shutting odwn glfw
  glfwDestroyWindow(mainWindow_);
  mainWindow_ = nullptr;
  glfwTerminate();

  return true;
}
