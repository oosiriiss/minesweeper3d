#include "glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <logzy/logzy.hpp>

#include "math.h"
#include "math.hpp"

struct Vec2 {
  float x;
  float y;
};

struct Vec3 {
  float x;
  float y;
  float z;
};

typedef struct Vertex {
  Vec2 pos;
  Vec3 col;
} Vertex;

static const Vertex vertices[3] = {{{-0.6f, -0.4f}, {1.f, 0.f, 0.f}},
                                   {{0.6f, -0.4f}, {0.f, 1.f, 0.f}},
                                   {{0.f, 0.6f}, {0.f, 0.f, 1.f}}};

static const char *vertexShaderText =
    "#version 330\n"
    "uniform mat4 MVP;\n"
    "in vec3 vCol;\n"
    "in vec2 vPos;\n"
    "out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    color = vCol;\n"
    "}\n";

static const char *fragmentShaderText = "#version 330\n"
                                        "in vec3 color;\n"
                                        "out vec4 fragment;\n"
                                        "void main()\n"
                                        "{\n"
                                        "    fragment = vec4(color, 1.0);\n"
                                        "}\n";

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

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
  glCompileShader(vertexShader);

  const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
  glCompileShader(fragmentShader);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  const GLint mvpLocation = glGetUniformLocation(program, "MVP");
  const GLint vposLocation = glGetAttribLocation(program, "vPos");
  const GLint vcolLocation = glGetAttribLocation(program, "vCol");

  GLuint vertexArray;

  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);
  glEnableVertexAttribArray(vposLocation);
  glVertexAttribPointer(vposLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, pos));
  glEnableVertexAttribArray(vcolLocation);
  glVertexAttribPointer(vcolLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, col));
  while (!glfwWindowShouldClose(window)) {

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float ratio = static_cast<float>(width) / height;

    glClear(GL_COLOR_BUFFER_BIT);

    double time = glfwGetTime();

    m4x4 m = m4x4::identity(1.0f);
    m.print();
    m = rotateZ(m, 10 * toRadians(static_cast<float>(time)));
    // m.print();
    m4x4 p = orthoProjection(-1.0f, 1.0f, -1.0f, 1.0f);
    p.print();

    m4x4 mvp = (p * m);
    mvp.print();

    glUseProgram(program);
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat *)&mvp.data);
    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  // Shutting odwn glfw
  glfwTerminate();

  return 0;
}
