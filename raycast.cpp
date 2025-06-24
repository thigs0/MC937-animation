#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include "hpp/raycast.hpp"
#include <glm/glm.hpp>

//intersecção raio triãngulo
bool rayTriangleIntersect(const glm::vec3& orig, const glm::vec3& d,
                          const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                          float& t, float& u, float& v) {
    const float EPSILON = 1e-2f; //critério de pequeno
    //Encontra as arestas
    glm::vec3 e1 = v1 - v0; //Edge 1
    glm::vec3 e2 = v2 - v0; // Edge 2
    glm::vec3 p = glm::cross(d, e2);
    float det = glm::dot(e1, p);

    // Verifica se o determinante é próximo de zero
    if (fabs(det) < EPSILON) return false;
    float invDet = 1.0f / det;

    
    glm::vec3 tvec = orig - v0;
    u = glm::dot(tvec, p) * invDet;
    if (u < 0.0f || u > 1.0f) return false; //verifica se u está fora do intervalo [0, 1]

    glm::vec3 qvec = glm::cross(tvec, e1);
    v = glm::dot(d, qvec) * invDet;
    if (v < 0.0f || u + v > 1.0f) return false; // verifica se v está fora do intervalo [0, 1] ou se u + v > 1

    // Se t > EPSILON -> raio atinge o triângulo
    t = glm::dot(e2, qvec) * invDet;
    return t > EPSILON;
}
