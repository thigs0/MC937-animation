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
#include <filesystem> // Para criar diretórios

#include "AABB.hpp"
#include "physics.hpp"  
#include "hpp/obj_loader.hpp"
#include "hpp/materials.hpp"

// Shaders
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5; // Ajuste conforme necessário
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 é o shininess
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Variáveis da cena
PhysicalObject box;
Cloth cloth;
PhysicalObject ground;

// Passo
float dt = 0.1f;

// Material
template<typename T> T clamp01(T v) { return v < T(0) ? T(0) : (v > T(1) ? T(1) : v); }
Material boxMaterial = gold;

// Luz
glm::vec3 lightPos(5.0f, 10.0f, 5.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

int width = 800, height = 600;

// COnfigurações de câmera
glm::vec3 cameraPos    = {-5.0f, 7.0f, -5.0f};
glm::vec3 cameraTarget = {0.0f,  0.0f,  0.0f};
glm::vec3 cameraUp     = {0.0f,  -1.0f,  0.0f};
glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
float aspect = float(width) / float(height);
glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

// CHão
std::vector<glm::vec3> groundVerts = {
    {-10.0f,0.0f,-10.0f}, {10.0f,0.0f,-10.0f}, {10.0f,0.0f,10.0f},
    {-10.0f,0.0f,-10.0f}, {10.0f,0.0f,10.0f},  {-10.0f,0.0f,10.0f}
};
std::vector<glm::vec3> groundNormals(groundVerts.size(), glm::vec3(0.0f,1.0f,0.0f));

// Lida com as colisões com a caixa e com o chão
void resolveCollisions(Cloth& C, const AABB& box, float floorY=0.0f, float restitution=0.3f, int iterations=3) {
    const float epsilon = 1e-3f;
    const float penThreshold = 0.5f;
    const float frictionCoeff = 0.9f;
    
    for (int iter = 0; iter < iterations; ++iter) {
        for (int i = 0; i < (int)C.positions.size(); ++i) {

            auto& p = C.positions[i];
            auto& v = C.velocities[i];
            
            // Calcula o quanto passou a bounding box da caixa pra cada direção
            if (box.contains(p)) {
                float dtc[6] = {
                    box.max_corner.y - p.y, // Topo
                    p.y - box.min_corner.y,  // Base
                    p.x - box.min_corner.x, // Esquerda
                    box.max_corner.x - p.x, // Direita
                    p.z - box.min_corner.z, // Frente
                    box.max_corner.z - p.z // Trás
                };

                // Verifica se está dentro da caixa
                int face = std::min_element(dtc, dtc+6) - dtc;
                
                // Para cada face, verifica se a distância é menor que o limiar de penetração. Se for menor, corrige a posição
                if (dtc[face] < penThreshold) {
                    switch(face) {
                        case 0: p.y = box.max_corner.y + epsilon; v.y *= -restitution; break;
                        case 1: p.y = box.min_corner.y - epsilon; v.y *= -restitution; break;
                        case 2: p.x = box.min_corner.x + epsilon; v.x *= -restitution; break;
                        case 3: p.x = box.max_corner.x + epsilon; v.x *= -restitution; break;
                        case 4: p.z = box.min_corner.z + epsilon; v.z *= -restitution; break;
                    }
                    
                    // Reduz a velocidade
                    v.x *= frictionCoeff;
                    v.z *= frictionCoeff;
                }
            }
            
            // Verifica colisão com o chão
            if (p.y < floorY + epsilon) {
                p.y = floorY + epsilon;
                v.y *= -restitution;
                v.x *= frictionCoeff;
                v.z *= frictionCoeff;
            }
        }
    }
}

// Acha os pontos mais próximos entre duas arestas
bool closestPointsBetweenEdges(
    const glm::vec3& p1, const glm::vec3& p2,  
    const glm::vec3& p3, const glm::vec3& p4,  
    glm::vec3& c1, glm::vec3& c2)              // Pontos c1 e c2
{

    glm::vec3 d1 = p2 - p1;
    glm::vec3 d2 = p4 - p3;
    glm::vec3 r = p1 - p3;
    
    // Calcula os dot products
    float a = glm::dot(d1, d1);
    float b = glm::dot(d1, d2);
    float c = glm::dot(d2, d2);
    float d = glm::dot(d1, r);
    float e = glm::dot(d2, r);
    
    // Calcula o determinante
    float det = a*c - b*b;
    
    if (det < 1e-6f) return false; // Se o determinante é muito pequeno, as arestas são consideradas paralelas
    
    // Calcula os pontos mais próximos e retorna true 

    float s = (b*e - c*d) / det;
    float t = (a*e - b*d) / det;
    
    s = glm::clamp(s, 0.0f, 1.0f);
    t = glm::clamp(t, 0.0f, 1.0f);
    
    c1 = p1 + d1 * s;
    c2 = p3 + d2 * t;
    
    return true;
}

// Resolve colisões de arestas
void resolveEdgeCollisions(Cloth& C, const AABB& box, float restitution=0.3f, float thickness=0.1f) {
    

    std::vector<std::pair<glm::vec3, glm::vec3>> boxEdges = {
        // Arestas da base
        {box.min_corner, glm::vec3(box.max_corner.x, box.min_corner.y, box.min_corner.z)},
        {box.min_corner, glm::vec3(box.min_corner.x, box.min_corner.y, box.max_corner.z)},
        {glm::vec3(box.max_corner.x, box.min_corner.y, box.min_corner.z), 
         glm::vec3(box.max_corner.x, box.min_corner.y, box.max_corner.z)},
        {glm::vec3(box.min_corner.x, box.min_corner.y, box.max_corner.z), 
         glm::vec3(box.max_corner.x, box.min_corner.y, box.max_corner.z)},
        
        //Verticais
        {box.min_corner, glm::vec3(box.min_corner.x, box.max_corner.y, box.min_corner.z)},
        {glm::vec3(box.max_corner.x, box.min_corner.y, box.min_corner.z), 
         glm::vec3(box.max_corner.x, box.max_corner.y, box.min_corner.z)},
        {glm::vec3(box.min_corner.x, box.min_corner.y, box.max_corner.z), 
         glm::vec3(box.min_corner.x, box.max_corner.y, box.max_corner.z)},
        {glm::vec3(box.max_corner.x, box.min_corner.y, box.max_corner.z), 
         glm::vec3(box.max_corner.x, box.max_corner.y, box.max_corner.z)},
        
        // Superiores
        {glm::vec3(box.min_corner.x, box.max_corner.y, box.min_corner.z), box.max_corner},
        {glm::vec3(box.min_corner.x, box.max_corner.y, box.min_corner.z), 
         glm::vec3(box.min_corner.x, box.max_corner.y, box.max_corner.z)},
        {glm::vec3(box.max_corner.x, box.max_corner.y, box.min_corner.z), box.max_corner},
        {glm::vec3(box.min_corner.x, box.max_corner.y, box.max_corner.z), box.max_corner}
    };
    
    const float collisionEps = thickness * 1.1f;
    
    // Para cada aresta no tecido, verifica se colide com as arestas da caixa e resolve se necessário
    for (const auto& clothEdge : C.edges) {
        const glm::vec3& p1 = C.positions[clothEdge.first];
        const glm::vec3& p2 = C.positions[clothEdge.second];
        
        for (const auto& boxEdge : boxEdges) {
            glm::vec3 c1, c2;
            // Verifica os pontos mais próximos entre a aresta do tecido e a aresta da caixa
            if (closestPointsBetweenEdges(p1, p2, boxEdge.first, boxEdge.second, c1, c2)) {
                glm::vec3 delta = c1 - c2;
                float dist = glm::length(delta);
                
                // Verifica se a distância entre os pontos mais próximos for menor que o epsilon de colisão
                if (dist < collisionEps) {
                    glm::vec3 normal = glm::normalize(delta);
                    float pen = collisionEps - dist;
                    
                    // Corrige a posição dos nós do tecido
                    glm::vec3 hc = normal * (pen * 0.5f);
                    C.positions[clothEdge.first]  += hc;
                    C.positions[clothEdge.second] -= hc;

                    // Corrige a velocidade
                    glm::vec3 relVel = C.velocities[clothEdge.second] - C.velocities[clothEdge.first];
                    float velNormal = glm::dot(relVel, normal);
                    if (velNormal < 0.0f) {
                 
                        float j_total = -(1.0f + restitution) * velNormal;

                        float j_half = j_total * 0.5f;
                        glm::vec3 impulso = j_half * normal;
                        C.velocities[clothEdge.first]  -= impulso;
                        C.velocities[clothEdge.second] += impulso;
                    }
                }
            }
        }
    }
}

GLuint createVAO(const std::vector<glm::vec3>& verts, const std::vector<glm::vec3>& norms) {
    GLuint VAO, VBO[2];
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(glm::vec3), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, norms.size()*sizeof(glm::vec3), norms.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return VAO;
}

// externals: shader program and uniform locations
GLuint shaderProgram;
GLint modelLoc, viewLoc, projLoc, objectColorLoc;
GLint lightPosLoc, lightColorLoc, viewPosLoc; // Adicionadas as localizações para iluminação

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " box.obj nFaces\n";
        return -1;
    }

    std::string objFIle   = argv[1];
    int nFaces        = std::atoi(argv[2]); // Quantidade da faces no tcido

    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* win = glfwCreateWindow(width, height, "", nullptr, nullptr);
    glfwMakeContextCurrent(win);
    glewInit(); glEnable(GL_DEPTH_TEST);

    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program." << std::endl;
        glfwDestroyWindow(win);
        glfwTerminate();
        return -1;
    }


    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projLoc = glGetUniformLocation(shaderProgram, "projection");
    objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");


    // carrega objeto
    loadOBJ(objFIle.c_str(), &box);
    glm::vec3 bmin = box.vertices[0], bmax = bmin;
    for (auto& v : box.vertices) {
        bmin = glm::min(bmin, v);
        bmax = glm::max(bmax, v);
    }
    AABB boxAABB(bmin, bmax);

    // Cria VAOs
    GLuint groundVAO = createVAO(groundVerts, groundNormals);
    GLuint boxVAO    = createVAO(box.vertices, box.normals);

    // Cria tecido
    createCloth(cloth, nFaces, 0.05f, 3.0f);
    cloth.mass = 0.5f;
    
    // VAO/VBO do tecido
    GLuint clothVAO, clothPosVBO, clothNormalVBO, clothEBO;
    std::vector<GLuint> edgeIdx;
    edgeIdx.reserve(cloth.edges.size()*2);
    for (auto&e: cloth.edges){ edgeIdx.push_back(e.first); edgeIdx.push_back(e.second);}   
    
    glGenVertexArrays(1, &clothVAO);
    glBindVertexArray(clothVAO);
    
    glGenBuffers(1, &clothPosVBO);
    glBindBuffer(GL_ARRAY_BUFFER, clothPosVBO);
    glBufferData(GL_ARRAY_BUFFER, cloth.positions.size()*sizeof(glm::vec3), cloth.positions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<glm::vec3> clothNormals(cloth.positions.size(), glm::vec3(0.0f, 1.0f, 0.0f)); // Normais iniciais
    glGenBuffers(1, &clothNormalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, clothNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, clothNormals.size()*sizeof(glm::vec3), clothNormals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &clothEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIdx.size()*sizeof(GLuint), edgeIdx.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);


    glUseProgram(shaderProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));



// produção da cena
for (int frame = 0; frame < 100; ++frame) {
    
    // Cria fundo
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Cria o chão
    glm::mat4 M_ground = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(M_ground));
    glUniform3f(objectColorLoc, 0.0f, 1.0f, 0.0f);
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)groundVerts.size());

    // Cria a caixa
    glm::mat4 M_box = glm::translate(glm::mat4(1.0f), glm::vec3(box.position));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(M_box));
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(boxMaterial.diffuse));
    glBindVertexArray(boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)box.vertices.size());

    // Simula o tecido
    int substeps = 3; //
    float substepDt = dt / substeps;

    for (int i = 0; i < substeps; ++i) {
        integrateCloth(cloth, substepDt); // Calcula o movimento do tecido
        resolveCollisions(cloth, boxAABB, 0.0f, 0.2f, 3); // Cuida das colisões com o chão e com a caixa
        resolveEdgeCollisions(cloth, boxAABB, 0.3f, 0.1f); // Cuida dos casos em que as arestas cortam a caixa
    }

    // Atualiza o tecido
    glBindBuffer(GL_ARRAY_BUFFER, clothPosVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cloth.positions.size()*sizeof(glm::vec3), cloth.positions.data());

    std::vector<glm::vec3> clothNormalsUpdated(cloth.positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));
    glBindBuffer(GL_ARRAY_BUFFER, clothNormalVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, clothNormalsUpdated.size()*sizeof(glm::vec3), clothNormalsUpdated.data());

    // Cria o tecido
    glm::mat4 M_cloth = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(M_cloth));
    glUniform3f(objectColorLoc, 0.7f, 0.2f, 0.2f);
    glBindVertexArray(clothVAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_LINES, (GLsizei)edgeIdx.size(), GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Frame atual
    std::vector<unsigned char> pixels(width*height*3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::ostringstream oss;
    oss << "./frame/scene2/frame" << std::setw(3) << std::setfill('0') << frame << ".png";
    std::cout << "Saving frame: " << oss.str() << std::endl;
    stbi_write_png(oss.str().c_str(), width, height, 3, pixels.data(), width*3);
}

    // Cleanup
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteVertexArrays(1, &boxVAO);
    glDeleteVertexArrays(1, &clothVAO);
    glDeleteBuffers(1, &clothPosVBO);
    glDeleteBuffers(1, &clothNormalVBO); 
    glDeleteBuffers(1, &clothEBO);
    glDeleteProgram(shaderProgram);


    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}