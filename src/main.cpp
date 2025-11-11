#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <logzy/logzy.hpp>
#include <string_view>
#include <utility>

#include "camera.hpp"
#include "math.hpp"
#include "program.hpp"
#include "shader.hpp"

static const Vertex vertices[] = {
    // Front face of cube - Red
    {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    //
    {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    // Back face of cube - Green
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    //
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    // Floor - Blue
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    //
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    // Ceil - Yellow
    {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    //
    {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    // Left wall - Purple
    {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
    //
    {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
    // Right wall - White
    {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
    //
    {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},

};

static constexpr auto verticesCount = sizeof(vertices) / sizeof(vertices[0]);

constexpr static std::string_view vertexShaderText = R"""(
#version 330
in vec3 vCol;
in vec3 vPos;
in vec3 vOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 color;
void main() {
    gl_Position = projection * view * model * vec4(vPos + vOffset, 1.0);
    color = vCol;
};
)""";

constexpr static std::string_view fragmentShaderText = R"""(
#version 330
in vec3 color;
out vec4 fragment;
void main() {
    fragment = vec4(color, 1.0);
};
)""";

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

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Generating cube offsets
  constexpr v3 dim{.x = 10, .y = 10, .z = 10};
  constexpr std::size_t cubesTotal =
      static_cast<std::size_t>(dim.x * dim.y * dim.z);

  std::array<v3, cubesTotal> cubes{};

  float spacing = 3.0f;

  for (int x = 0; x < dim.x; ++x) {
    for (int y = 0; y < dim.y; ++y) {
      for (int z = 0; z < dim.z; ++z) {
        cubes[x * 100 + y * 10 + z] =
            v3{.x = spacing * x, .y = spacing * y, .z = spacing * z};
      }
    }
  }

  GLuint instanceBuffer;
  glGenBuffers(1, &instanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubes), cubes.data(), GL_STATIC_DRAW);

  std::optional<Program> programOpt = Program::create(std::vector{
      std::pair{vertexShaderText, Shader::Type::Vertex},
      std::pair{fragmentShaderText, Shader::Type::Fragment},
  });

  Program &program = *programOpt;

  GLint vposLocation = -1;
  GLint voffsetLocation = -1;
  GLint vcolLocation = -1;

  if (auto vposLocationOpt = program.getAttribLocation("vPos")) {
    vposLocation = *vposLocationOpt;
  } else {
    return -1;
  }
  if (auto vcolLocationOpt = program.getAttribLocation("vCol")) {
    vcolLocation = *vcolLocationOpt;
  } else {
    return -1;
  }
  if (auto voffsetLocationOpt = program.getAttribLocation("vOffset")) {
    voffsetLocation = *voffsetLocationOpt;
  } else {
    return -1;
  }

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);

  glBindVertexArray(vertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glEnableVertexAttribArray(vposLocation);
  glVertexAttribPointer(vposLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, position));
  glEnableVertexAttribArray(vcolLocation);
  glVertexAttribPointer(vcolLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, color));

  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  glEnableVertexAttribArray(voffsetLocation);
  glVertexAttribPointer(voffsetLocation, 3, GL_FLOAT, GL_FALSE, sizeof(v3),
                        nullptr);
  glVertexAttribDivisor(voffsetLocation, 1);

  constexpr v3 cameraInitialPosition{.z = 20.0f};
  constexpr v3 cameraArbitraryUp{.y = 1.0F};
  constexpr float cameraSpeed = 10.0F;
  constexpr float horizontalSensitivity = 0.8f;
  constexpr float verticalSensitivity = horizontalSensitivity / 1.5f;

  Camera camera(cameraInitialPosition, cameraArbitraryUp);

  v2d mousePos;
  glfwGetCursorPos(window, &mousePos.x, &mousePos.y);

  double lastTime = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double time = static_cast<float>(glfwGetTime());

    // Handling mouse movement
    v2d newMousePos;
    glfwGetCursorPos(window, &newMousePos.x, &newMousePos.y);
    v2d mouseDelta = newMousePos - mousePos;

    // TODO :: Investigate why does yaw have to be negated in order to rotate in
    // the right direction
    camera.rotate(
        {.x = static_cast<float>(mouseDelta.y * horizontalSensitivity),
         .y = static_cast<float>(-mouseDelta.x * verticalSensitivity),
         .z = 0.0f});

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

    m4x4 m = m4x4::identity(1.0f);
    // m = translate(m, v3{.x = -2.f, .y = -0.5F, .z = -5.f});
    m = rotate(m, radians(50.0F * static_cast<float>(time)), v3{.y = 1.0f});
    m = scale(m, v3{.x = 0.1f, .y = 0.1f, .z = 0.1f});

    const m4x4 &v = camera.getView();
    auto p = perspective();

    program.use();
    program.setM4x4("model", m);
    program.setM4x4("view", v);
    program.setM4x4("projection", p);

    glBindVertexArray(vertexArray);
    // glDrawArrays(GL_TRIANGLES, 0, verticesCount);
    glDrawArraysInstanced(GL_TRIANGLES, 0, verticesCount, cubes.size());

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  // Shutting odwn glfw
  glfwTerminate();

  return 0;
}
