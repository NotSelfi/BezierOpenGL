cmake_minimum_required(VERSION 3.10)
project(BezierOpenGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(external/glfw)

include_directories(
    include
    external/glad/include
    external/glfw/include
    external/glm
)

add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_executable(BezierOpenGL
    src/main.cpp
    src/BezierCurveData.cpp
)

target_link_libraries(BezierOpenGL glfw glad)
