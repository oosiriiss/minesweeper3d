cmake_minimum_required(VERSION 3.19)

project(minesweeper3d)



include(FetchContent)
FetchContent_Declare(
   logzy
   GIT_REPOSITORY https://github.com/oosiriiss/logzy
   GIT_TAG 0.2.6
)
FetchContent_MakeAvailable(logzy)


add_executable(
   minesweeper3d

   ./vendor/glad/glad.c

   ./src/main.cpp

   ./src/math.hpp

   ./src/shader.cpp
   ./src/shader.hpp

   ./src/program.cpp
   ./src/program.hpp
)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# TODO Turn off building examples once base is set up
set(GLFW_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)



add_subdirectory(./vendor/glfw)

include_directories(./vendor/glad)
include_directories(./vendor/glfw)
include_directories(./vendor/stb)

find_package(OpenGL REQUIRED)

# TODO :: GLM is only for now so I dont wanna bother setting up dependency properly
find_package(glm REQUIRED)

target_compile_features(minesweeper3d PUBLIC cxx_std_23)

target_link_libraries(minesweeper3d glfw OpenGL::GL logzy::logzy glm::glm)




