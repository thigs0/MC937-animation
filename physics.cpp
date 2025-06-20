#include <iostream>
#include <glm/glm.hpp>
#include "hpp/materials.hpp" //predefinição de alguns materiais
#include <map>
#include <vector>
#include "hpp/physics.hpp"
#include <cmath>
#include <string>

// Usando glm::vec3 para vetores 3D
using Vec3 = glm::dvec3;  // double precision vec3

const double AIR_DENSITY = 1.225;
const Vec3 G = Vec3(0.0, -9.81, 0.0);

void update_ambient_forces(PhysicalObject* obj, double dt) {
    Vec3 gforce = G * obj->mass;

    double speed = glm::length(obj->velocity);
    Vec3 airf(0.0);

    if (speed > 0.0) {
        Vec3 vhat = glm::normalize(obj->velocity);
        airf = vhat * (-0.5 * AIR_DENSITY * speed * speed * obj->dragCoefficient * obj->frontalArea);
    }

    Vec3 totalForce = gforce + airf;
    Vec3 acceleration = totalForce / obj->mass;

    obj->velocity += acceleration * dt;
    obj->position += obj->velocity * dt;
}


void applyTornadoForce(PhysicalObject* obj, glm::vec3 center, double dt) {
    glm::vec3 pos = glm::vec3(obj->position);
    glm::vec3 toCenter = center - pos;
    float dist = glm::length(toCenter);

    if (dist < 0.00001f) return; // evitar divisão por zero

    // Vetor perpendicular à direção para o centro (gira no plano XZ)
    glm::vec3 spiralDir = glm::normalize(glm::cross(toCenter, glm::vec3(0, 1, 0)));

    // Parâmetros ajustáveis
    float maxRadius = 6.0f;          // Raio máximo permitido
    float inwardStrength = 10.0f;    // Força que puxa para o centro
    float spiralStrength = 10.0f;    // Força de rotação
    float liftStrength = 10.0f;      // Força vertical

    glm::vec3 inwardForce = glm::normalize(toCenter) * inwardStrength;
    glm::vec3 spiralForce = spiralDir * spiralStrength;
    glm::vec3 upwardForce = glm::vec3(0, liftStrength, 0);

    glm::vec3 totalForce = inwardForce + spiralForce + upwardForce;

    // Se estiver muito longe do centro, aplica força extra para puxar de volta
    if (dist > maxRadius) {
        glm::vec3 pullBack = glm::normalize(toCenter) * (dist - maxRadius) * 20.0f;
        totalForce += pullBack;
    }

    // Aplica força na velocidade
    obj->velocity += glm::dvec3(totalForce) * dt;

    // Freio radial (reduz velocidade para fora do tornado)
    glm::vec3 vel = glm::vec3(obj->velocity);
    glm::vec3 radialDir = glm::normalize(toCenter);
    float radialSpeed = glm::dot(vel, -radialDir);  // velocidade saindo do centro
    if (radialSpeed > 0.0f) {
        glm::vec3 brakingForce = radialDir * radialSpeed * 2.0f;
        obj->velocity -= glm::dvec3(brakingForce) * dt;
    }
}

/// Cria uma malha de tecido e armazena em obj
void createCloth(PhysicalObject* obj, int n_faces) {
    if (!obj || n_faces <= 0) return;

    // Limpa qualquer dado anterior
    obj->vertices.clear();
    obj->normals.clear();
    obj->faces.clear();
    obj->materials.clear();

    // Calcular dimensões do grid a partir de n_faces (2 triângulos por quadrado)
    int quads = n_faces / 2;
    int grid_size = std::sqrt(quads);
    if (grid_size * grid_size != quads) {
        // Ajusta para próximo valor quadrado se necessário
        grid_size = std::ceil(std::sqrt(quads));
    }

    float spacing = 1.0f;

    // Gerar vértices: grid (grid_size + 1) x (grid_size + 1)
    for (int y = 0; y <= grid_size; ++y) {
        for (int x = 0; x <= grid_size; ++x) {
            glm::vec3 position = glm::vec3(x * spacing, 0.0f, y * spacing);
            obj->vertices.push_back(position);
            obj->normals.push_back(glm::vec3(0, 1, 0));  // Normais iniciais para cima
        }
    }

    // Gerar faces (2 triângulos por quadrado)
    for (int y = 0; y < grid_size; ++y) {
        for (int x = 0; x < grid_size; ++x) {
            int i = y * (grid_size + 1) + x;
            int i_right = i + 1;
            int i_down = i + (grid_size + 1);
            int i_diag = i_down + 1;

            // Triângulo 1
            obj->faces.push_back(Face({(unsigned int)i, (unsigned int)i_right, (unsigned int)i_down}, {}, "cloth"));
            obj->faces.push_back(Face({(unsigned int)i_right, (unsigned int)i_diag, (unsigned int)i_down}, {}, "cloth"));

        }
    }

    // Define material básico
    Material clothMaterial;
    clothMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    clothMaterial.diffuse = glm::vec3(0.7f, 0.7f, 0.9f);
    clothMaterial.specular = glm::vec3(0.1f, 0.1f, 0.1f);
    clothMaterial.shininess = 16.0f;

    obj->materials["cloth"] = clothMaterial;
}

