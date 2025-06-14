cmake_minimum_required(VERSION 3.10)
project(MC937Animation)

# Use C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0072 NEW)

# Onde está o seu código-fonte
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

# 1) Pacotes externos
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)

# 2) Diretórios de include
#    - OpenGL, GLFW, GLEW, OpenCV
#    - sua pasta stb local
include_directories(
    ${CMAKE_SOURCE_DIR}          
    ${CMAKE_SOURCE_DIR}/stb
    ${CMAKE_SOURCE_DIR}/hpp
    ${OpenCV_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

add_library(collision STATIC
    ${CMAKE_SOURCE_DIR}/AABB.cpp
)
add_library(physics STATIC
    ${CMAKE_SOURCE_DIR}/physics.cpp
)
add_library(raycast STATIC
    ${CMAKE_SOURCE_DIR}/raycast.cpp
)

target_include_directories(physics PUBLIC
    ${CMAKE_SOURCE_DIR}/hpp
)

target_include_directories(collision PUBLIC
    ${CMAKE_SOURCE_DIR}/hpp
)
add_library(loader STATIC
    ${CMAKE_SOURCE_DIR}/obj_loader.cpp
)

# 3) Executável
add_executable(scene1
    scene1.cpp
)

add_executable(scene2
    scene2.cpp
)

add_executable(scene3
    scene3.cpp
)


target_include_directories(scene1 PRIVATE
    ${CMAKE_SOURCE_DIR}/hpp
)

# 4) Linkagem
target_link_libraries(scene1
    collision
    physics
    raycast
    loader
    ${OpenCV_LIBS}
    OpenGL::GL
    glfw
    GLEW::GLEW
)

target_link_libraries(scene2
    collision
    physics
    raycast
    loader
    ${OpenCV_LIBS}
    OpenGL::GL
    glfw
    GLEW::GLEW
)

target_link_libraries(scene3
    collision
    physics
    raycast
    loader
    ${OpenCV_LIBS}
    OpenGL::GL
    glfw
    GLEW::GLEW
)

# 5) Mensagens de debug (opcional)
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "STB include:   ${PROJECT_ROOT}/stb")
