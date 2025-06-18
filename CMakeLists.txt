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

# ImGui setup
add_library(imgui
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        src/Mesh.cpp
)

target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/backends
)

add_library(glad external/glad/src/glad.c
        src/Mesh.cpp)

add_executable(BezierOpenGL
        src/main.cpp
        src/BezierCurveData.cpp
        src/Extrusion.cpp
        src/Camera.cpp
        src/Mesh.cpp
)

target_link_libraries(BezierOpenGL glfw glad imgui)

# === Platform stuff ===
if (WIN32)
    target_link_libraries(BezierOpenGL opengl32)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
    target_link_libraries(BezierOpenGL ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKIT_LIBRARY} ${CoreVideo_LIBRARY})
else()
    target_link_libraries(BezierOpenGL GL dl pthread X11 Xxf86vm Xrandr Xi Xcursor)
endif()