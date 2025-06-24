#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>  // Necessário para glm::dvec3
#include "hpp/materials.hpp"
#include <map>

//fwefwefgwerg
using Vec3 = glm::dvec3; 

extern const Vec3 G;
extern const double AIR_DENSITY;

double norm(const Vec3& v);
Vec3 hat(const Vec3& a);

struct Face {
    std::vector<unsigned int> vertex_indices;
    std::vector<unsigned int> normal_indices;
    std::string material_name;
};

struct Cloth {
  std::vector<glm::vec3> positions;       // N partículas
  std::vector<glm::vec3> velocities;      // N velocidades
  std::vector<std::pair<int,int>> edges;  
  std::vector<float> restLengths;        
  float mass = 0.9f;
};

struct PhysicalObject {
    double mass;
    double dragCoefficient;
    double frontalArea;

    glm::dvec3 position;
    glm::dvec3 velocity;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;
    std::map<std::string, Material> materials;
};

void update_ambient_forces(PhysicalObject* obj, double dt);
void applyTornadoForce(PhysicalObject* obj, glm::vec3 center, double dt);
void createCloth(Cloth &cloth, int nFaces,
                 float spacing,
                 float initialHeight);

void computeSpringForces(const Cloth& C, std::vector<glm::vec3>& F);
void applyExternalForces(const Cloth& C, std::vector<glm::vec3>& F);
void integrateCloth(Cloth& C, float dt);

#endif // PHYSICS_HPP