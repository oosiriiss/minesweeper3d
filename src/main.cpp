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

  v3 cameraPosition{.z = 20.0f};
  v3 cameraTarget{};
  v3 cameraUp{.x = 0.0f, .y = 1.0f, .z = 0.0f};
  v3 cameraRight{.x = 1.0f, .y = 0.0f, .z = 0.0f};

  double lastTime = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double time = glfwGetTime();

    double dt = lastTime - time;
    lastTime = time;

    if (glfwGetKey(window, GLFW_KEY_A)) {
      cameraPosition.x += dt * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
      cameraPosition.x -= dt * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_W)) {
      cameraPosition.z += dt * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
      cameraPosition.z -= dt * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
      cameraPosition.y -= dt * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
      cameraPosition.y += dt * 10.0f;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float ratio = static_cast<float>(width) / height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m4x4 m = m4x4::identity(1.0f);
    // m = translate(m, v3{.x = -2.f, .y = -0.5F, .z = -5.f});
    m = rotate(m, toRadians(50.0F * static_cast<float>(time)), v3{.y = 1.0f});
    m = scale(m, v3{.x = 0.1f, .y = 0.1f, .z = 0.1f});

    m4x4 v = lookAt(cameraPosition, cameraTarget, cameraUp, cameraRight);

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
