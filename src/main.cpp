#include "glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <logzy/logzy.hpp>

#include "math.hpp"

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

static const char *vertexShaderText =
    "#version 330\n"
    "uniform mat4 MVP;\n"
    "in vec3 vCol;\n"
    "in vec3 vPos;\n"
    "out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
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
  glEnable(GL_DEPTH_TEST);

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

    m4x4 m = m4x4::identity(1.0f);
    //m = rotateZ(m, 50 * toRadians(static_cast<float>(time)));
    //logzy::info("after Z");
    //m.print();
    m = rotateY(m, 50 * toRadians(static_cast<float>(time)));
    logzy::info("after Y");
    m.print();
    logzy::info("After translation");
    m = translate(m, v3{.x = 0, .y = 0, .z = -10.0f});
    m.print();
    m4x4 p = orthoProjection(-5.0f, 5.0f, -5.0f, 5.0f);

    logzy::info("MVP");
    m4x4 mvp = (p * m);
    mvp.print();

    glUseProgram(program);
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat *)&mvp.data);
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
