# Versão mínima do CMake necessária
cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(main)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar o OpenCV instalado no sistema
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)

# Mensagem de status para verificar se o OpenCV foi encontrado
message(STATUS "OpenCV library status:")
message(STATUS "    Version: ${OpenCV_VERSION}")
message(STATUS "    Libraries: ${OpenCV_LIBS}")
message(STATUS "    Include path: ${OpenCV_INCLUDE_DIRS}")

# Adicionar o executável
add_executable(main main.cpp)

INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} )

# Linkar o OpenCV ao projeto
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main OpenGL::GL)
target_link_libraries(main glfw)
target_link_libraries(main GLEW::GLEW)
