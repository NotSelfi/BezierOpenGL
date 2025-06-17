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

add_library(glad external/glad/src/glad.c
        src/Extrusion.cpp
        include/Extrusion.hpp
        include/Mesh.hpp
        src/Camera.cpp
)
target_include_directories(glad PUBLIC external/glad/include)

add_executable(BezierOpenGL
    src/main.cpp
    src/BezierCurveData.cpp
        src/Extrusion.cpp
        include/Extrusion.hpp
        include/Mesh.hpp
        include/Camera.hpp
        src/Camera.cpp
)

target_link_libraries(BezierOpenGL glfw glad)
