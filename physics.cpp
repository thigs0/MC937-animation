#include <iostream>
#include <glm/glm.hpp>

// Usando glm::vec3 para vetores 3D
using Vec3 = glm::dvec3;  // double precision vec3

const Vec3 G = Vec3(0.0, -9.8, 0.0);
const double AIR_DENSITY = 1.225;

struct PhysicalObject {
    double mass;                // kg
    double dragCoefficient;     // adimensional
    double frontalArea;         // m²
    
    Vec3 position;              // m
    Vec3 velocity;              // m/s
};

void update_ambient_forces(PhysicalObject* obj, double dt) {
    Vec3 gforce = G * obj->mass;

    double speed = glm::length(obj->velocity);
    Vec3 airf(0.0);

    if (speed > 0.0) {
        Vec3 vhat = glm::normalize(obj->velocity);
        // Força de arrasto: F = -0.5 * ρ * v² * Cd * A * v̂
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


