#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include "hpp/materials.hpp" //predefinição de alguns materiais
#include <map>
#include <vector>
#include "hpp/physics.hpp"
#include <cmath>
#include <string>
#include <algorithm>

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

// Cria o tecido
void createCloth(Cloth &cloth, int nFaces,
                 float spacing = 1.0f,
                 float initialHeight = 10.0f) {
    if (nFaces <= 0) return;

    // Limpa dados
    cloth.positions.clear();
    cloth.velocities.clear();
    cloth.edges.clear();
    cloth.restLengths.clear();

    // Calcular o grid do tecido, considerando que cada face é um quadrado
    int quads = nFaces / 2;
    int gridSize = static_cast<int>(std::sqrt(quads));
    if (gridSize * gridSize < quads) {
        gridSize = static_cast<int>(std::ceil(std::sqrt(quads)));
    }

    // Calcular o deslocamento para centralizar o tecido na cena
    float offsetX = (gridSize * spacing) / 2.0f;
    float offsetZ = (gridSize * spacing) / 2.0f; 

    // Inicializa as posições 
    for (int y = 0; y <= gridSize; ++y) {
        for (int x = 0; x <= gridSize; ++x) {
            cloth.positions.emplace_back(x * spacing - offsetX, // Centraliza no X
                                         initialHeight,
                                         y * spacing - offsetZ); // Centraliza no Z
        }
    }

    // veloc inicial = 0
    cloth.velocities.assign(cloth.positions.size(), glm::vec3(0.0f));

    // 5) COnstrói as arestas do tecido
    auto idx = [gridSize](int x, int y) {
        return y * (gridSize + 1) + x;
    };
    for (int y = 0; y <= gridSize; ++y) {
        for (int x = 0; x <= gridSize; ++x) {
            // horizontal neighbor
            if (x < gridSize) {
                cloth.edges.emplace_back(idx(x,y), idx(x+1,y));
            }
            // vertical neighbor
            if (y < gridSize) {
                cloth.edges.emplace_back(idx(x,y), idx(x,y+1));
            }
        }
    }

    cloth.restLengths.reserve(cloth.edges.size());
    for (auto &e : cloth.edges) {
        int i = e.first;
        int j = e.second;
        float rest = glm::length(cloth.positions[j] - cloth.positions[i]);
        cloth.restLengths.push_back(rest);
    }
}


// Determina o damping e a força elástica para cada aresta do tecido
void computeSpringForces(const Cloth& C, std::vector<glm::vec3>& F) {
  float ks=100.f, kd=1.5f;
  int M = C.edges.size();
  for(int k=0; k<M; ++k){
    auto [i,j] = C.edges[k];
    glm::vec3 L = C.positions[j] - C.positions[i];
    float len = glm::length(L);
    glm::vec3 dir = L/len;
    glm::vec3 vrel = C.velocities[j] - C.velocities[i];
    glm::vec3 F_s = -ks*(len - C.restLengths[k])*dir;
    glm::vec3 F_d = -kd*(glm::dot(vrel,dir))*dir;
    F[i] -= (F_s+F_d);
    F[j] += (F_s+F_d);
  }
}

// Aplica gravidade no tecido (vento removido)
void applyExternalForces(const Cloth& C, std::vector<glm::vec3>& F) {
  int N=C.positions.size();
  glm::vec3 g(0,-0.98f,0);
  for(int i=0;i<N;++i){
    F[i] += g * C.mass;
    // wind randômico
    // F[i] += glm::vec3(
    //   glm::linearRand(-0.1f,0.1f),
    //   glm::linearRand(-0.1f,0.1f),
    //   glm::linearRand(-0.1f,0.1f)
    // );
  }
}

// Integrador RK4
void integrateCloth(Cloth& C, float dt){
  int N=C.positions.size();
  std::vector<glm::vec3> F(N), a(N);
  computeSpringForces(C, F);
  applyExternalForces(C, F);
  for(int i=0;i<N;++i){
    a[i] = F[i] / C.mass;
    C.velocities[i] += a[i] * dt;
    C.positions[i]  += C.velocities[i] * dt;
  }
}


