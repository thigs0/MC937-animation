#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include "AABB.hpp"

const float gravity = -9.8f;

struct Velocity {
    glm::vec3 position;
    float vy;
    float minY;
};

struct PhysObj {
    glm::vec3 position;
    float     vy;
    AABB      bbox;      // bounding box em espaço local
};

void updatePhysics(PhysObj& obj1, float gravity,float dt) {
    // 1) Integra gravidade e posição
    obj1.vy += gravity * dt;
    obj1.position.y += obj1.vy * dt;

    // 2) Translada a AABB para o espaço de mundo
    AABB world_box = obj1.bbox;
    world_box.min_corner += obj1.position;
    world_box.max_corner += obj1.position;

    // 3) Teste de colisão com o chão (y=0)
    if (world_box.min_corner.y < 0.0f) {
        // 3a) Ajusta position para que a caixa fique “encostada” no chão
        float penetration = -world_box.min_corner.y;
        obj1.position.y += penetration;

        // 3b) Recalculamos a AABB depois do push-up
        world_box.min_corner.y += penetration;
        world_box.max_corner.y += penetration;

        // 3c) “Quica” invertendo vy com amortecimento
        obj1.vy = -obj1.vy * 0.8f;
        
        // 3d) Se o quique for muito pequeno, zera
        if (std::abs(obj1.vy) < 0.1f) obj1.vy = 0.0f;
    }
}

// Recebe um arquivo .obj, o procesa e coloca todos os vértices na referência out_vertices
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
            unsigned int vi1;
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


float getMinY(const std::vector<glm::vec3>& verts) {
    float minY = verts[0].y;
    for (const auto& v : verts) {
        if (v.y < minY) minY = v.y;
    }
    return minY;
}

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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: ./render modelo1.obj" << std::endl;
        return -1;
    }

    //std::filesystem::create_directories("frame/");

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // janela invisível
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Cria uma janela 800x600
    GLFWwindow* window = glfwCreateWindow(800, 600, "", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;
    glEnable(GL_DEPTH_TEST);

    std::vector<glm::vec3> verts1;
    if (!loadSimpleOBJ(argv[1], verts1)) {
        std::cerr << "Falha ao carregar o OBJ!\n";
        return -1;
    }


    // Faz a bounding box da malha
    glm::vec3 mesh_min = verts1[0], mesh_max = verts1[0];
    for (auto& v : verts1) {
        mesh_min = glm::min(mesh_min, v);
        mesh_max = glm::max(mesh_max, v);
    }
    AABB bbox_local(mesh_min, mesh_max);

    PhysObj obj1 { glm::vec3(-3, 23, 0), 0.0f, bbox_local };

    std::cout << "Vertices carregados: " << verts1.size() << "\n";
    if (verts1.empty()) {
        std::cerr << "Nenhum vértice válido encontrado no OBJ!\n";
        return -1;
    }

    GLuint VAO1 = createVAO(verts1);
    
    GLuint shaderProgram = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragment_shader_src);
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc  = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc  = glGetUniformLocation(shaderProgram, "projection");

    glm::mat4 view = glm::lookAt(glm::vec3(9.0f, 9.0f, 9.0f),
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            800.0f / 600.0f,
                                            0.1f, 100.0f);

    // float minY1 = getMinY(verts1); // Pega o vértice com o menor y do modelo 
    // Velocity obj1 { glm::vec3(-3.0f, 10.0f, 0.0f), 0.0f, minY1 };


    for (int frame = 0; frame < 300; ++frame) {

        std::cout << "Creating frame number:" << frame << std::endl;

        // Define a posição em y do objeto no frame
        updatePhysics(obj1, gravity,0.1);

        // Reseta a imagem anterior
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model1 = glm::translate(glm::mat4(1.0f), obj1.position);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, verts1.size());

        std::vector<unsigned char> pixels(800 * 600 * 3);
        glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        for (int y = 0; y < 300; ++y) {
            for (int x = 0; x < 800 * 3; ++x) {
                std::swap(pixels[y * 800 * 3 + x], pixels[(599 - y) * 800 * 3 + x]);
            }
        }
        
        std::ostringstream oss;
        oss << "./frame/scene1/" << std::setw(3) << std::setfill('0') << frame << ".png";
        stbi_write_png(oss.str().c_str(), 800, 600, 3, pixels.data(), 800 * 3);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

