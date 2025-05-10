#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

// Função simples para carregar arquivos OBJ
bool loadSimpleOBJ(const std::string& filename, std::vector<glm::vec3>& out_vertices) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir arquivo: " << filename << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<unsigned int> vertexIndices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        } else if (prefix == "f") {
            unsigned int vi1, vi2, vi3;
            char slash; // Ignora '/'
            std::string vert;
            for (int i = 0; i < 3; i++) {
                iss >> vert;
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::istringstream viss(vert);
                viss >> vi1;
                vertexIndices.push_back(vi1 - 1);
            }
        }
    }

    for (unsigned int idx : vertexIndices) {
        out_vertices.push_back(temp_vertices[idx]);
    }

    return true;
}

const char* vertex_shader_src = R"(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

const char* fragment_shader_src = R"(
#version 330 core
out vec4 fragColor;
void main() {
    fragColor = vec4(1.0, 0.6, 0.2, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint createVAO(const std::vector<glm::vec3>& vertices) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    return VAO;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Uso: ./loader modelo1.obj modelo2.obj modelo3.obj" << std::endl;
        return -1;
    }

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Multi OBJ Viewer", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;
    glEnable(GL_DEPTH_TEST);

    // Carrega os 3 arquivos
    std::vector<glm::vec3> verts1, verts2, verts3;
    if (!loadSimpleOBJ(argv[1], verts1)) return -1;
    if (!loadSimpleOBJ(argv[2], verts2)) return -1;
    if (!loadSimpleOBJ(argv[3], verts3)) return -1;

    GLuint VAO1 = createVAO(verts1);
    GLuint VAO2 = createVAO(verts2);
    GLuint VAO3 = createVAO(verts3);

    GLuint shaderProgram = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragment_shader_src);
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    //Camera
    glm::mat4 view = glm::lookAt(glm::vec3(9.0f, 9.0f, 9.0f),
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Agora, itera sobre os três modelos, ajustando as posições
        glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, verts1.size());

        glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, verts2.size());

        glm::mat4 model3 = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
        glBindVertexArray(VAO3);
        glDrawArrays(GL_TRIANGLES, 0, verts3.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

