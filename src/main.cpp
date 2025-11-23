#include "game/board.hpp"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <logzy/logzy.hpp>

#include "math/matrix.hpp"
#include "render/camera.hpp"

// static const Vertex vertices[] = {
//     // Front face of cube - Red
//     {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0F, 0.0F}}, // BL
//     {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0F, 1.0F}},  // TL
//     {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0F, 1.0F}},   // TR
//     //
//     {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0F, 0.0F}}, // BL
//     {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0F, 1.0F}},   // TR
//     {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0F, 0.0F}},  // BR
//     // Back face of cube - Green
//     {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0F, 0.0F}}, // BL
//     {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0F, 1.0F}},  // TL
//     {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0F, 1.0F}},   // TR
//     //
//     {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0F, 0.0F}}, // BL
//     {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0F, 1.0F}},   // TR
//     {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0F, 0.0F}},  // BR
//     // Floor - Blue
//     {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0F, 1.0F}},
//     {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0F, 0.0F}},
//     {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0F, 0.0F}},
//     //
//     {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0F, 1.0F}},
//     {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0F, 1.0F}},
//     {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0F, 0.0F}},
//     // Ceil - Yellow
//     {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}, {0.0F, 1.0F}},
//     {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0F, 0.0F}},
//     {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0F, 0.0F}},
//     //
//     {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}, {0.0F, 1.0F}},
//     {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}, {1.0F, 1.0F}},
//     {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0F, 0.0F}},
//     // Left wall - Purple
//     {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0F, 0.0F}},
//     {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0F, 0.0F}},
//     {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0F, 1.0F}},
//     //
//     {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0F, 0.0F}},
//     {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0F, 1.0F}},
//     {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0F, 1.0F}},
//     // Right wall - White
//     {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0F, 0.0F}},
//     {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0F, 0.0F}},
//     {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0F, 1.0F}},
//     //
//     {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0F, 0.0F}},
//     {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0F, 1.0F}},
//     {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0F, 1.0F}},
// };

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

  // GLuint vertexBuffer;
  // glGenBuffers(1, &vertexBuffer);
  // glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Generating cube offsets
  // constexpr v3f dim = vec3<float>(10.0F, 10.0F, 10.0F);
  // constexpr std::size_t cubesTotal =
  //    static_cast<std::size_t>(dim.x() * dim.y() * dim.z());

  // std::array<v3f, cubesTotal> cubes{};

  // float spacing = 3.0f;

  // for (int x = 0; x < dim.x(); ++x) {
  //   for (int y = 0; y < dim.y(); ++y) {
  //     for (int z = 0; z < dim.z(); ++z) {
  //       cubes[x * 100 + y * 10 + z] =
  //           vec3<float>(spacing * x, spacing * y, spacing * z);
  //     }
  //   }
  // }

  // GLuint instanceBuffer;
  // glGenBuffers(1, &instanceBuffer);
  // glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(cubes), cubes.data(), GL_STATIC_DRAW);

  // Texture
  // v2i textureDim;
  // std::int32_t channels;
  // const char *texturePath{"assets/container.jpg"};

  //// Making sure the texture isnt upside down
  // stbi_set_flip_vertically_on_load(true);

  // unsigned char *data =
  //     stbi_load(texturePath, &textureDim.x(), &textureDim.y(), &channels, 0);

  // if (data == nullptr) {
  //   logzy::critical(
  //       "Couldn't find file: {}. Make sure it is in the executable's
  //       directory", texturePath);
  //   return -1;
  // }

  // logzy::info("Loaded texture. Dimensions: {}x{} and channels: {}",
  //             textureDim.x(), textureDim.y(), channels);

  // GLuint texture;
  // glGenTextures(1, &texture);
  // glBindTexture(GL_TEXTURE_2D, texture);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
  //                 GL_LINEAR_MIPMAP_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureDim.x(), textureDim.y(), 0,
  //              GL_RGB, GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);

  // Freeing the image data as it is already loaded onto GPU.
  // stbi_image_free(data);

  // std::optional<Program> programOpt = Program::create(std::vector{
  //     std::pair{vertexShaderText, Shader::Type::Vertex},
  //     std::pair{fragmentShaderText, Shader::Type::Fragment},
  // });

  // Program &program = *programOpt;

  // GLint vposLocation = -1;
  // GLint voffsetLocation = -1;
  // GLint vcolLocation = -1;
  // GLint textureLocation = -1;

  // logzy::info("Checking attribute locations");

  // if (auto vposLocationOpt = program.getAttribLocation("vPos")) {
  //   vposLocation = *vposLocationOpt;
  // } else {
  //   return -1;
  // }
  // logzy::info("vposLocation found");
  // if (auto vcolLocationOpt = program.getAttribLocation("vCol")) {
  //   vcolLocation = *vcolLocationOpt;
  // } else {
  //   return -1;
  // }
  // logzy::info("vcolLocation found");
  // if (auto voffsetLocationOpt = program.getAttribLocation("vOffset")) {
  //   voffsetLocation = *voffsetLocationOpt;
  // } else {
  //   return -1;
  // }

  // logzy::info("voffsetLocation found");
  // if (auto textureLocationOpt =

  //        program.getAttribLocation("vTextureCoordinate")) {
  //  textureLocation = *textureLocationOpt;
  //} else {
  //  return -1;
  //}
  // logzy::info("vtextureLocation found");
  // logzy::info("All attribute locations found");

  // GLuint vertexArray;
  // glGenVertexArrays(1, &vertexArray);

  // glBindVertexArray(vertexArray);
  // glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

  // glVertexAttribPointer(vposLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
  //                       (void *)offsetof(Vertex, position));
  // glEnableVertexAttribArray(vposLocation);

  // glVertexAttribPointer(vcolLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
  //                       (void *)offsetof(Vertex, color));
  // glEnableVertexAttribArray(vcolLocation);

  // glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE,
  // sizeof(Vertex),
  //                       (void *)offsetof(Vertex, texture));
  // glEnableVertexAttribArray(textureLocation);

  // glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  // glEnableVertexAttribArray(voffsetLocation);
  // glVertexAttribPointer(voffsetLocation, 3, GL_FLOAT, GL_FALSE, sizeof(v3f),
  //                       nullptr);
  // glVertexAttribDivisor(voffsetLocation, 1);

  Board board;

  if (auto boardOpt = Board::create(vec3<std::int32_t>(5, 5, 5))) {
    board = std::move(*boardOpt);
  } else {
    logzy::critical("Couldn't create board");
    return -1;
  }

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
