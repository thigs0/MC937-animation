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
#include <random>

#include "AABB.hpp"
#include "physics.hpp"  // Deve conter PhysicalObject e update_ambient_forces()
#include "hpp/obj_loader.hpp"
#include "hpp/raycast.hpp" //implementação do raycast
#include "hpp/materials.hpp" //predefinição de alguns materiais

PhysicalObject homer;
PhysicalObject ground;

glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f); // defina o centro do tornado conforme necessário
double deltaTime = 0.016; // ou o valor real do seu timestep/frame

float dt = 0.0001; //variação de tempo
int mframe = 100;
Material m = gold; // mapa de cor do ouro
glm::vec3 lightPos(5, 10, 5), lightColor(1, 0.1, 1); //cor branca global
//viewport
int width = 800;
int height = 600;


struct Pixel {
    unsigned char r, g, b;
};
//configurações da câmera
glm::vec3 cameraPos   = glm::vec3(10.0f, 20.0f, 10.0f);
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

void updatePhysics(PhysObj& obj1, double dt, PhysicalObject *homer) {
    std::cout << "Homer before: ("
            << homer->position.x << ", "
            << homer->position.y << ", "
            << homer->position.z << ")\n";
    applyTornadoForce(homer, origin, dt);
    homer->position += homer->velocity * dt;
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

bool checkAABBCollision(const AABB& a, const glm::vec3& posA,
                        const AABB& b, const glm::vec3& posB) {
    return (a.min_corner.x + posA.x <= b.max_corner.x + posB.x &&
            a.max_corner.x + posA.x >= b.min_corner.x + posB.x &&
            a.min_corner.y + posA.y <= b.max_corner.y + posB.y &&
            a.max_corner.y + posA.y >= b.min_corner.y + posB.y &&
            a.min_corner.z + posA.z <= b.max_corner.z + posB.z &&
            a.max_corner.z + posA.z >= b.min_corner.z + posB.z);
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

AABB computeAABB(const std::vector<glm::vec3>& vertices) {
    glm::vec3 minV = vertices[0], maxV = vertices[0];
    for (const auto& v : vertices) {
        minV = glm::min(minV, v);
        maxV = glm::max(maxV, v);
    }
    return AABB(minV, maxV);
}

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

    GLFWwindow* window = glfwCreateWindow(width, height, "", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);

    if (!loadOBJ(modeloPath, &homer)) {
        std::cerr << "Falha ao carregar modelo.\n";
        return -1;
    }

    // Calcula a bounding box do modelo
    AABB bbox = computeAABB(homer.vertices);

    // Gera os nObjetos em posições aleatórias ao redor da origem
    std::vector<PhysicalObject> objetos;
    std::vector<PhysObj> objboxs;
    objetos.reserve(nObjetos);
    objboxs.reserve(nObjetos);

    glm::vec3 mesh_min = homer.vertices[0], mesh_max = homer.vertices[0];
    for (auto& v : homer.vertices) {
        mesh_min = glm::min(mesh_min, v);
        mesh_max = glm::max(mesh_max, v);
    }
    AABB bbox_local(mesh_min, mesh_max);
    PhysObj obj1 { glm::vec3(-3, 23, 0), 0.0f, bbox_local }; 


    std::default_random_engine rng;
    std::uniform_real_distribution<float> distrib(-10.0f, 10.0f);

    for (int i = 0; i < nObjetos; ++i) {
        PhysicalObject obj = homer;  // Cópia do objeto
        obj.position = glm::vec3(distrib(rng), distrib(rng), distrib(rng));
        PhysObj tbox { obj.position, 0.0f, bbox_local };
        objetos.push_back(obj);
        objboxs.push_back(tbox);
    }

    GLuint VAO = createVAO(homer.vertices, homer.normals);
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

    for (int frame=0; frame < mframe; ++frame) {

        std::cout << "Frame " << frame << "\n";

        for (int i = 0; i < nObjetos; ++i) {
            std::cout << "Caclulate physics obj " << i << "\n";
            updatePhysics(objboxs[i], dt, &objetos[i]);
            objetos[i].position = objboxs[i].position;//sincroniza box com objeto
            // Verifica colisão com todos os outros objetos
            std::cout << "Checking colision obj " << i << "\n";
            for (int j = i + 1; j < nObjetos; ++j) {
                if (checkAABBCollision(objboxs[i].bbox, objetos[i].position,
                                    objboxs[j].bbox, objetos[j].position)) {

                    // Vetor entre os centros
                    glm::vec3 dir = objetos[j].position - objetos[i].position;
                    if (glm::length(dir) < 1e-5f) dir = glm::vec3(1.0f, 0.0f, 0.0f);
                    dir = glm::normalize(dir);

                    // Reposicionamento leve
                    float push = 0.05f;
                    objetos[i].position -= dir * push;
                    objetos[j].position += dir * push;

                    // Inversão das velocidades como resposta simplificada
                    std::swap(objetos[i].velocity, objetos[j].velocity);
                    objboxs[i].position = objetos[i].position;
                    objboxs[j].position = objetos[j].position;
                }
            }
        }

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glm::vec3 hitColor(0.0f);
        std::vector<Pixel> framebuffer(width * height);

        std::cout << "Building scene " << "\n";

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 invViewProj = glm::inverse(projection * view);

        glm::vec3 eye = cameraPos;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {

                std::cout << "Raycasting pixel (" << x << ", " << y << ")\n";

                // Calcula o raio a partir da posição da câmera
                float ndcX = (2.0f * x) / width - 1.0f;
                float ndcY = 1.0f - (2.0f * y) / height;
                glm::vec4 ndc = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
                glm::vec4 worldRay4 = invViewProj * ndc;
                worldRay4 /= worldRay4.w;

                // Normaliza o vetor de direção do raio
                float px = (2.0f * (x + 0.5f) / width - 1.0f) * imagePlaneWidth * 0.5f;
                float py = (1.0f - 2.0f * (y + 0.5f) / height) * imagePlaneHeight * 0.5f;
                glm::vec3 pixelPos = cameraPos + forward + px * right + py * camUp;
                glm::vec3 dir = glm::normalize(pixelPos - cameraPos);

                glm::vec3 rayDir = glm::normalize(glm::vec3(worldRay4) - eye);
                glm::vec3 rayOrigin = eye;

                float minDist = 1e20f;

                // Verifica interseção com o chão
                float closestT = 1e30f;
                glm::vec3 hitPoint, hitNormal;
                Material hitMat = gold;
                for (int i = 0; i < nObjetos; ++i) {
                    const auto& obj = objetos[i];
                    for (size_t j = 0; j < obj.vertices.size(); j += 3) {
                        // Vértices do triângulo com transformação de posição do objeto
                        glm::vec3 v0 = obj.vertices[j + 0]     +  glm::vec3(obj.position);
                        glm::vec3 v1 = obj.vertices[j + 1] +  glm::vec3(obj.position);
                        glm::vec3 v2 = obj.vertices[j + 2] +  glm::vec3(obj.position);

                        glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(obj.position));
                        glm::vec3 v0w = glm::vec3(model1 * glm::vec4(v0, 1.0f));
                        glm::vec3 v1w = glm::vec3(model1 * glm::vec4(v1, 1.0f));
                        glm::vec3 v2w = glm::vec3(model1 * glm::vec4(v2, 1.0f));

                        glm::vec3 hitPos, normal;
                        float t, u, v;

                        // Verifica interseção do raio com o triângulo
                        if (rayTriangleIntersect(cameraPos, dir, v0w, v1w, v2w, t, u, v)) {
                            if (t < closestT) {
                                closestT = t;
                                hitPoint = cameraPos + dir * t;

                                // Normais interpoladas
                                glm::vec3 n0 = obj.normals[j];
                                glm::vec3 n1 = obj.normals[j + 1];
                                glm::vec3 n2 = obj.normals[j + 2];
                                hitNormal = glm::normalize((1 - u - v) * n0 + u * n1 + v * n2);
                            }
                        }
                        if (closestT < 1e30f) {
                            hitMat = gold; // ou bronze, ou silver, se quiser variar

                            hitColor = computeColor(hitPoint, hitNormal, lightPos, lightColor, hitMat);
                            hitColor = glm::clamp(hitColor, 0.0f, 1.0f);
                        } else {
                            hitColor = glm::vec3(0.1f, 0.1f, 0.3f); // cor de fundo
                        }

                    }
                }


                hitColor = glm::clamp(hitColor, 0.0f, 1.0f);
                framebuffer[y * width + x] = {
                    static_cast<unsigned char>(hitColor.r * 255),
                    static_cast<unsigned char>(hitColor.g * 255),
                    static_cast<unsigned char>(hitColor.b * 255)
                };
            }
        }

        std::cout << "Saving..." << "\n";
        // Salvar imagem
        std::ostringstream oss;
        oss << "./frame/scene3/" << std::setw(3) << std::setfill('0') << frame << ".png";
        stbi_write_png(oss.str().c_str(), width, height, 3, framebuffer.data(), width * 3);

    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
