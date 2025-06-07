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
#include "physics.hpp"  // Deve conter PhysicalObject e update_ambient_forces()
#include "hpp/obj_loader.hpp"

// Objetos físicos globais
PhysicalObject homer;
PhysicalObject h2;
PhysicalObject h3;

struct PhysObj {
    glm::vec3 position;
    float     vy;
    AABB      bbox;      // bounding box em espaço local
};
// TO DO: Add -> Forca que puxe para o centro
// TO DO: Fazer com que ambos tenham o mesmo chao
// Atualiza a física individualmente para cada PhysObj e seu respectivo PhysicalObject
void updatePhysics(PhysObj& obj1, PhysicalObject& physObj, double dt) {
  glm::vec3 tornadoCenter  = {0.0,0.0,0.0};
    update_ambient_forces(&physObj, dt);
    applyTornadoForce(&physObj, tornadoCenter, dt);

    obj1.position = glm::vec3(physObj.position);
    obj1.vy = static_cast<float>(physObj.velocity.y);

    AABB world_box = obj1.bbox;
    world_box.min_corner += obj1.position;
    world_box.max_corner += obj1.position;

    // Colisão chão
    if (world_box.min_corner.y < 0.0f) {
        float penetration = -world_box.min_corner.y;
        obj1.position.y += penetration;
        physObj.position.y += penetration;
        if (physObj.velocity.y < 0)
            physObj.velocity.y = -physObj.velocity.y * 0.8;

        world_box.min_corner.y += penetration;
        world_box.max_corner.y += penetration;

        if (std::abs(physObj.velocity.y) < 0.1f) {
            physObj.velocity.y = 0.0;
            obj1.vy = 0.0f;
        }
    }
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

// OBJ loader simples (igual ao original)
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

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
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
    if (argc < 3) {
        std::cerr << "Uso: ./render modelo.obj N_objetos\n";
        return -1;
    }

    std::string modeloPath = argv[1];
    int nObjetos = std::stoi(argv[2]);
    if (nObjetos <= 0) {
        std::cerr << "Número inválido de objetos.\n";
        return -1;
    }

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);

    std::vector<glm::vec3> vertices;
    if (!loadSimpleOBJ(modeloPath, vertices)) {
        std::cerr << "Falha ao carregar modelo.\n";
        return -1;
    }

    AABB bbox = [&vertices]() {
        glm::vec3 minV = vertices[0], maxV = vertices[0];
        for (const auto& v : vertices) {
            minV = glm::min(minV, v);
            maxV = glm::max(maxV, v);
        }
        return AABB(minV, maxV);
    }();

    GLuint VAO = createVAO(vertices);
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
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f/600.f, 0.1f, 100.f);

    // Inicializa objetos físicos
    std::vector<PhysicalObject> físicos(nObjetos);
    std::vector<PhysObj> objetos(nObjetos);

    // Para cada objeto
    for (int i = 0; i < nObjetos; ++i) {
        //Define um posicao inicial aleatoria
        float x = static_cast<float>(rand() % 1000 - 500) / 100.0f; // -5 a 5
        float z = static_cast<float>(rand() % 1000 - 500) / 100.0f;
        float y = 2.0f + static_cast<float>(rand() % 300) / 100.0f; // 2 a 5

        //Define as propriedades fisicas
        físicos[i].mass = 20.0 + (rand() % 100) / 10.0;
        físicos[i].position = glm::dvec3(x, y, z);
        físicos[i].velocity = glm::dvec3(0.0);

        //define o objeto como instancia de PhysObj
        objetos[i] = { glm::vec3(físicos[i].position), 0.0f, bbox };
    }

    for (int frame = 0; frame < 50; ++frame) {

        std::cout << "Frame " << frame << "\n";

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (int i = 0; i < nObjetos; ++i) {

            updatePhysics(objetos[i], físicos[i], 0.01);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), objetos[i].position);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }

        // Salvar frame
        std::vector<unsigned char> pixels(800 * 600 * 3);
        glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        for (int y = 0; y < 300; ++y)
            for (int x = 0; x < 800 * 3; ++x)
                std::swap(pixels[y * 800 * 3 + x], pixels[(599 - y) * 800 * 3 + x]);

        std::ostringstream oss;
        oss << "./frame/scene3/" << std::setw(3) << std::setfill('0') << frame << ".png";
        stbi_write_png(oss.str().c_str(), 800, 600, 3, pixels.data(), 800 * 3);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
