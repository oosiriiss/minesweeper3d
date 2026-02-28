cmake_minimum_required(VERSION 3.19)

option(COMPILE_TESTS "Whether tests will be compiled" ON)

if(NOT COMPILE_TESTS) 
   return()
endif()

project(minesweeper3d-tests)

include(FetchContent)
FetchContent_Declare(
   tasty
   GIT_REPOSITORY https://github.com/oosiriiss/tasty
   GIT_TAG 0.2.9.2
)
FetchContent_MakeAvailable(tasty)



function(Register_test name)
      add_executable(${name} ${name}.cpp)
      target_link_libraries(${name} PRIVATE minesweeper3d-lib tasty::tasty)
      target_compile_features(${name} PRIVATE cxx_std_23)
      add_test(NAME ${name} COMMAND ${name})
endfunction()

Register_test(example_test)

message("Tests registered")
