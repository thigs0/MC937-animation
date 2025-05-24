cmake_minimum_required(VERSION 3.10)
project(MC937Animation)

# Use C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

target_include_directories(collision PUBLIC
    ${CMAKE_SOURCE_DIR}/hpp
)

# 3) Executável
add_executable(scene1
    scene1.cpp
)

target_include_directories(scene1 PRIVATE
    ${CMAKE_SOURCE_DIR}/hpp
)

# 4) Linkagem
target_link_libraries(scene1
    collision
    ${OpenCV_LIBS}
    OpenGL::GL
    glfw
    GLEW::GLEW
)

# 5) Mensagens de debug (opcional)
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "STB include:   ${PROJECT_ROOT}/stb")
