cmake_minimum_required(VERSION 3.19)

option(ENABLE_DEBUG_UTILS "Enable debug_utils.hpp" ON)

project(minesweeper3d)



include(FetchContent)
FetchContent_Declare(
   logzy
   GIT_REPOSITORY https://github.com/oosiriiss/logzy
   GIT_TAG 0.2.6
)
FetchContent_MakeAvailable(logzy)

add_library(
   minesweeper3d-lib

   ./src/game/board.cpp
   ./src/game/board.hpp
   ./src/game/crosshair.cpp
   ./src/game/crosshair.hpp

   ./src/resource_manager.cpp
   ./src/resource_manager.hpp

   ./src/math/math.hpp
   ./src/math/matrix.hpp

   ./src/render/camera.hpp
   ./src/render/mesh.hpp
   ./src/render/program.cpp
   ./src/render/program.hpp
   ./src/render/shader.cpp
   ./src/render/shader.hpp
   ./src/render/texture.cpp
   ./src/render/texture.hpp

   ./src/application.cpp
   ./src/application.cpp
)

add_executable(minesweeper3d
   ./src/main.cpp
   ./vendor/glad/glad.c
)

if(ENABLE_DEBUG_UTILS)
   target_compile_definitions(minesweeper3d-lib PUBLIC ENABLE_DEBUG_UTILS)
   target_compile_definitions(minesweeper3d PUBLIC ENABLE_DEBUG_UTILS)
endif()

target_compile_features(minesweeper3d-lib PUBLIC cxx_std_23)
target_compile_features(minesweeper3d PUBLIC cxx_std_23)

include_directories(./src)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# TODO Turn off building examples once base is set up
set(GLFW_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)

add_subdirectory(./vendor/glfw)

include_directories(./vendor/glad)
include_directories(./vendor/glfw)
include_directories(./vendor/stb)
include_directories(./vendor/debug_utils)

find_package(OpenGL REQUIRED)

# TODO :: GLM is only for now so I dont wanna bother setting up dependency properly
find_package(glm REQUIRED)



target_link_libraries(minesweeper3d-lib glfw OpenGL::GL logzy::logzy glm::glm)
target_link_libraries(minesweeper3d minesweeper3d-lib)

add_subdirectory(./tests)




