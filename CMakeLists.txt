cmake_minimum_required(VERSION 3.19)

project(minesweeper3d)

# Including glad 
include_directories(./glad)


include(FetchContent)
FetchContent_Declare(
   logzy
   GIT_REPOSITORY https://github.com/oosiriiss/logzy
   GIT_TAG 0.2.6
)
FetchContent_MakeAvailable(logzy)


add_executable(
   minesweeper3d
   ./glad/glad.c

   ./src/main.cpp
)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# TODO Turn off building examples once base is set up
set(GLFW_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)

add_subdirectory(glfw)
find_package(OpenGL REQUIRED)

# TODO :: GLM is only for now so I dont wanna bother setting up dependency properly
find_package(glm REQUIRED)

target_link_libraries(minesweeper3d glfw OpenGL::GL logzy::logzy glm::glm)




