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
#include "hpp/raycast.hpp" //implementação do raycast
#include "hpp/materials.hpp" //predefinição de alguns materiais

// -----------------------Variáveis da cena-----------------------------------------------------------------------------------------
PhysicalObject box;
PhysicalObject cloth;
PhysicalObject ground;

float dt = 0.1; //variação de tempo
Material m = gold; // mapa de cor do ouro
glm::vec3 lightPos(5, 10, 5), lightColor(1, 0.1, 1); //cor branca global
//viewport
int width = 800;
int height = 600;


struct Pixel {
    unsigned char r, g, b;
};
//configurações da câmera
glm::vec3 cameraPos   = glm::vec3(5.0f, 10.0f, 5.0f);
glm::vec3 cameraTarget= glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

float aspect = static_cast<float>(width) / static_cast<float>(height);
glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

// Dimensões do plano da câmera
float imagePlaneWidth = 2.0f;  // Ajuste conforme FOV e aspect ratio
float imagePlaneHeight = 1.5f; // Idem

//glm::vec3 eye = glm::vec3(0.0f, 8.0f, 15.0f);
glm::vec3 forward = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - cameraPos);
glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
glm::vec3 camUp = glm::cross(right, forward);

//ground variables

std::vector<glm::vec3> groundVerts = {
        { -10.0f, 0.0f, -10.0f },  // X,    Y=0,  Z
        {  10.0f, 0.0f, -10.0f },
        {  10.0f, 0.0f,  10.0f },
        { -10.0f, 0.0f, -10.0f },
        {  10.0f, 0.0f,  10.0f },
        { -10.0f, 0.0f,  10.0f }
    };
std::vector<glm::vec3> groundNormals(groundVerts.size(), glm::vec3(0.0f, 1.0f, 0.0f));
//---------------------------------------------------------------------------------------------------------------------------------
struct PhysObj {
    glm::vec3 position;
    float     vy;
    AABB      bbox;      // bounding box em espaço local
};

void updatePhysics(PhysObj& obj1, double dt, PhysicalObject *homer)  {
    update_ambient_forces(homer, dt);

    // Atualiza a posição do PhysObj com a posição do homer (sincroniza)
    obj1.position = homer->position;
    obj1.vy = static_cast<float>(homer->velocity.y);
    //PERGUNTA: Aparentemente nao esta mais acelerando com a gravidade

    // Translada a AABB para o espaço de mundo
    AABB world_box = obj1.bbox;
    world_box.min_corner += obj1.position;
    world_box.max_corner += obj1.position;

    // Teste de colisão com o chão (y=0)
    if (world_box.min_corner.y < 0.0f) {
        // Ajusta position para que a caixa fique encostada no chão
        float penetration = -world_box.min_corner.y;
        obj1.position.y += penetration;

        // Atualiza o objeto homer também
        homer->position.y += penetration;
        if (homer->velocity.y < 0)
            homer->velocity.y = -homer->velocity.y * 0.8;

        // Recalcula a AABB após ajuste
        world_box.min_corner.y += penetration;
        world_box.max_corner.y += penetration;

        // Zera a velocidade vertical se muito pequena
        if (std::abs(homer->velocity.y) < 0.1f) {
            homer->velocity.y = 0.0;
            obj1.vy = 0.0f;
        }
    }
}

GLuint createVAO(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals) {
    GLuint VAO, VBOs[2];
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBOs);

    glBindVertexArray(VAO);

    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Normais — agora usa o parâmetro 'normals' passado corretamente!
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return VAO;
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}

glm::vec3 computeColor(const glm::vec3& point, const glm::vec3& normal,
                       const glm::vec3& lightPos, const glm::vec3& lightColor,
                       const Material& mat) {
    glm::vec3 ambient = mat.ambient * lightColor;
    glm::vec3 L = glm::normalize(lightPos - point);
    glm::vec3 N = glm::normalize(normal);
    glm::vec3 diffuse = mat.diffuse * glm::max(glm::dot(N, L), 0.0f) * lightColor;
    glm::vec3 V = glm::normalize(-point);
    glm::vec3 R = glm::reflect(-L, N);
    glm::vec3 specular = mat.specular * pow(glm::max(glm::dot(R, V), 0.0f), mat.shininess) * lightColor;
    return ambient + diffuse + specular;
}

float getMinY(const std::vector<glm::vec3>& verts) {
    float minY = verts[0].y;
    for (const auto& v : verts) {
        if (v.y < minY) minY = v.y;
    }
    return minY;
}
const char* vertex_shader_src = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

)";

const char* fragment_shader_src = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}

)";

int main(int argc, char** argv) {
    //define o chão
    ground.vertices = {
    { -10.0f, 0.0f, -10.0f },
    {  10.0f, 0.0f, -10.0f },
    {  10.0f, 0.0f,  10.0f },
    { -10.0f, 0.0f,  10.0f }
    };
    ground.normals = {
        { 0.0f, 1.0f, 0.0f }
    };
    ground.faces = {
        Face{ {0, 1, 2}, {0, 0, 0}, "" },  // Triângulo 1
        Face{ {0, 2, 3}, {0, 0, 0}, "" }   // Triângulo 2
    };

    //--------------------------------------------------------------------------
    if (argc < 2) {
        std::cerr << "Uso: ./render modelo1.obj" << std::endl;
        return -1;
    }

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // janela invisível
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;
    glEnable(GL_DEPTH_TEST);

    std::string objFilename = argv[1];

    if (!loadOBJ("./OBJ/out.obj", &box)) {
        std::cerr << "erro para carregar o arquivo ./OBJ/out.obj";
        return -1;
    }
    int nfaces = std::atoi(argv[1]);
    createCloth(&cloth, nfaces);// cria o tecido com n faces

    // Bounding box da malha
    box.mass = 1.0;
    box.position = glm::dvec3(5.0, 5.0, 0.0);
    box.velocity = glm::dvec3(0.0, -2.0, 0.0);
    glm::vec3 mesh_min = box.vertices[0], mesh_max = box.vertices[0];
    for (auto& v : box.vertices) {
        mesh_min = glm::min(mesh_min, v);
        mesh_max = glm::max(mesh_max, v);
    }
    AABB bbox_local(mesh_min, mesh_max);

    PhysObj obj1 { glm::vec3(-3, 23, 0), 0.0f, bbox_local }; 

    GLuint groundVAO = createVAO(ground.vertices, ground.normals);

    // antes do loop, crie VAO e shaders uma vez:

    GLuint VAO1 = createVAO(box.vertices, box.normals);

    GLuint shaderProgram = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragment_shader_src);
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint ColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

    for (int frame = 0; frame < 100; ++frame) {
        std::cout << "Creating frame number: " << frame << std::endl;

        // Atualiza física com delta time 0.1s
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 modelGround = glm::mat4(1.0f);              
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelGround));
        glUniform3f(ColorLoc, 0.0f, 1.0f, 0.0f); // Cor verde para o chão
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, groundVerts.size());

        // Render Homer na posição atual
        glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(box.position));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glUniform3fv(ColorLoc, 1, glm::value_ptr(m.diffuse));
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, box.vertices.size());

        // Captura frame e salva
        std::vector<unsigned char> pixels(800 * 600 * 3);
        glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        glm::vec3 lightPos(5, 5, 5);
        glm::vec3 lightColor(1, 1, 1);

        std::vector<unsigned char> framebuffer(width * height * 3); // all pixel with null value

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float px = (2.0f * (x + 0.5f) / width - 1.0f) * imagePlaneWidth * 0.5f;
                float py = (1.0f - 2.0f * (y + 0.5f) / height) * imagePlaneHeight * 0.5f;
                glm::vec3 pixelPos = cameraPos + forward + px * right + py * camUp;
                glm::vec3 dir = glm::normalize(pixelPos - cameraPos);

                float closestT = 1e30f;
                glm::vec3 hitPoint, hitNormal;
                Material hitMat = gold;

                for (const Face& f : cloth.faces) {
                    if (f.vertex_indices.size() < 3) continue;
                    

                    glm::vec3 v0 = cloth.vertices[f.vertex_indices[0]];
                    glm::vec3 v1 = cloth.vertices[f.vertex_indices[1]];
                    glm::vec3 v2 = cloth.vertices[f.vertex_indices[2]];

                    glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(cloth.position));
                    glm::vec3 v0w = glm::vec3(model1 * glm::vec4(v0, 1.0f));
                    glm::vec3 v1w = glm::vec3(model1 * glm::vec4(v1, 1.0f));
                    glm::vec3 v2w = glm::vec3(model1 * glm::vec4(v2, 1.0f));
                    float t, u, v;
                    if (rayTriangleIntersect(cameraPos, dir, v0w, v1w, v2w, t, u, v)) {
                        if (t < closestT) {
                            closestT = t;
                            hitPoint = cameraPos + dir * t;
                            glm::vec3 n0 = cloth.normals[f.normal_indices[0]];
                            glm::vec3 n1 = cloth.normals[f.normal_indices[1]];
                            glm::vec3 n2 = cloth.normals[f.normal_indices[2]];
                            hitNormal = glm::normalize((1 - u - v) * n0 + u * n1 + v * n2);
                            if (cloth.materials.count(f.material_name))
                                hitMat = cloth.materials[f.material_name];
                        }
                    }
                }

                glm::vec3 color = (closestT < 1e30f)
                                ? computeColor(hitPoint, hitNormal, lightPos, lightColor, hitMat)
                                : glm::vec3(0.0f, 0.7f, 1.0f);

                int idx = 3 * (y * width + x);
                framebuffer[idx + 0] = static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f); // R
                framebuffer[idx + 1] = static_cast<unsigned char>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f); // G
                framebuffer[idx + 2] = static_cast<unsigned char>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f); // B

            }
        }

        std::ostringstream oss;
        oss << "./frame/scene1/frame" << std::setw(3) << std::setfill('0') << frame << ".png";

        stbi_write_png(oss.str().c_str(), width, width, 3, framebuffer.data(), width * 3);
        std::cout << "Imagem salva em " << oss.str() << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

