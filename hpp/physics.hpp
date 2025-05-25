#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>  // Necessário para glm::dvec3

using Vec3 = glm::dvec3;

extern const Vec3 G;
extern const double AIR_DENSITY;

double norm(const Vec3& v);
Vec3 hat(const Vec3& a);

struct PhysicalObject {
    double mass;                  
    double dragCoefficient;       
    double frontalArea;           
    
    Vec3 position;            
    Vec3 velocity;            
};

void update_ambient_forces(PhysicalObject* obj, double dt);
void applyTornadoForce(PhysicalObject* obj, glm::vec3 center, double dt);

#endif // PHYSICS_HPP
