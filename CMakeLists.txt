cmake_minimum_required(VERSION 3.19)

option(ENABLE_DEBUG_UTILS "Enable debug_utils.hpp" ON)

project(minesweeper3d)



include(FetchContent)
FetchContent_Declare(
   logzy
   GIT_REPOSITORY https://github.com/oosiriiss/logzy
   GIT_TAG f78a0215f1c5733be803eecabe1e96c40f0e8375 # 0.1
)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG        7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4
)
FetchContent_MakeAvailable(logzy glfw)


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

   ./vendor/imgui/imconfig.h
   ./vendor/imgui/imgui.h
   ./vendor/imgui/imgui.cpp
   ./vendor/imgui/imgui_draw.cpp
   ./vendor/imgui/imgui_impl_glfw.cpp
   ./vendor/imgui/imgui_impl_glfw.h
   ./vendor/imgui/imgui_impl_opengl3.cpp
   ./vendor/imgui/imgui_impl_opengl3.h
   ./vendor/imgui/imgui_internal.h
   ./vendor/imgui/imgui_tables.cpp
   ./vendor/imgui/imgui_widgets.cpp
   ./vendor/imgui/imstb_rectpack.h
   ./vendor/imgui/imstb_textedit.h
   ./vendor/imgui/imstb_truetype.h
)

add_executable(minesweeper3d
   ./src/main.cpp
   ./vendor/glad/glad.c
)

if(ENABLE_DEBUG_UTILS)
   target_compile_definitions(minesweeper3d-lib PUBLIC ENABLE_DEBUG_UTILS)
   target_compile_definitions(minesweeper3d PUBLIC ENABLE_DEBUG_UTILS)
endif()

# This caused issues when includign glad
target_compile_definitions(minesweeper3d-lib PUBLIC GLFW_INCLUDE_NONE)

target_compile_features(minesweeper3d-lib PUBLIC cxx_std_23)
target_compile_features(minesweeper3d PUBLIC cxx_std_23)

include_directories(./src)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# TODO Turn off building examples once base is set up
set(GLFW_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)

include_directories(./vendor/glad)
include_directories(./vendor/glfw)
include_directories(./vendor/stb)
include_directories(./vendor/debug_utils)
include_directories(./vendor/imgui/)

find_package(OpenGL REQUIRED)



target_link_libraries(minesweeper3d-lib glfw OpenGL::GL logzy::logzy glfw)
target_link_libraries(minesweeper3d minesweeper3d-lib)


include(CTest)
enable_testing()
add_subdirectory(./tests)




