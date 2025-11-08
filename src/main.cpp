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
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    //
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    // Back face of cube - Green
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    //
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    // Floor - Blue
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    //
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    // Ceil - Yellow
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
    //
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
    // Left wall - Purple
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
    //
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
    // Right wall - White
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    //
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},

};

static constexpr auto verticesCount = sizeof(vertices) / sizeof(vertices[0]);

constexpr static std::string_view vertexShaderText = R"""(
#version 330
in vec3 vCol;
in vec3 vPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 color;
void main() {
    gl_Position = projection * view * model * vec4(vPos, 1.0);
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
    std::cout << "GLFW not intialized\n";
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 800, "My window", nullptr, nullptr);

  if (!window) {
    std::cout << "Couldn't create GLFW window\n";
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

  std::optional<Program> programOpt = Program::create(std::vector{
      std::pair{vertexShaderText, Shader::Type::Vertex},
      std::pair{fragmentShaderText, Shader::Type::Fragment},
  });

  Program &program = *programOpt;

  GLint vposLocation = -1;
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

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);

  glBindVertexArray(vertexArray);
  glEnableVertexAttribArray(vposLocation);
  glVertexAttribPointer(vposLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, position));
  glEnableVertexAttribArray(vcolLocation);
  glVertexAttribPointer(vcolLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, color));
  while (!glfwWindowShouldClose(window)) {

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float ratio = static_cast<float>(width) / height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double time = glfwGetTime();

    m4x4 m(scale(m4x4::identity(1.0f), v3{.x = 0.4f, .y = 0.4f, .z = 0.4f}));
    m = rotateY(m, 50 * toRadians(static_cast<float>(time)));
    m = rotateX(m, 50 * toRadians(static_cast<float>(time)));
    m = rotateZ(m, 50 * toRadians(static_cast<float>(time)));
    m4x4 v = translate(m4x4::identity(1.0f), v3{.x = 0, .y = 0, .z = -3.0f});
    auto p = perspective();

    glm::mat4 model(1.0f);

    model = glm::rotate(model, glm::radians(static_cast<float>(time) * 100.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    program.use();
    program.setM4x4("model", m);
    program.setM4x4("view", v);
    program.setM4x4("projection", p);

    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, verticesCount);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  // Shutting odwn glfw
  glfwTerminate();

  return 0;
}
