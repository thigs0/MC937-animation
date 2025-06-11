#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include <glm/glm.hpp>

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

// Ouro polido
const Material gold = {
    glm::vec3(0.24725f, 0.1995f, 0.0745f),
    glm::vec3(0.75164f, 0.60648f, 0.22648f),
    glm::vec3(0.628281f, 0.555802f, 0.366065f),
    51.2f
};
// Prata polida
const Material silver = {
    glm::vec3(0.19225f, 0.19225f, 0.19225f),
    glm::vec3(0.50754f, 0.50754f, 0.50754f),
    glm::vec3(0.508273f, 0.508273f, 0.508273f),
    51.2f
};

// Plástico fosco vermelho
const Material redMattePlastic = {
    glm::vec3(0.05f, 0.0f, 0.0f),
    glm::vec3(0.5f, 0.0f, 0.0f),
    glm::vec3(0.7f, 0.6f, 0.6f),
    10.0f
};
const Material blackMattePlastic = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.01f, 0.01f, 0.01f),
    glm::vec3(0.05f, 0.05f, 0.05f),
    10.0f
};
const Material blueMattePlastic = {
    glm::vec3(0.0f, 0.0f, 0.05f),
    glm::vec3(0.0f, 0.0f, 0.5f),
    glm::vec3(0.6f, 0.6f, 0.7f),
    10.0f
};
const Material greenMattePlastic = {
    glm::vec3(0.0f, 0.05f, 0.0f),
    glm::vec3(0.0f, 0.5f, 0.0f),
    glm::vec3(0.6f, 0.7f, 0.6f),
    10.0f
};




#endif // MATERIALS_HPP

