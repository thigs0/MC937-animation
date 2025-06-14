#ifndef RAYCAST_HPP
#define RAYCAST_HPP

#include <glm/glm.hpp>

bool rayTriangleIntersect(const glm::vec3& orig, const glm::vec3& d,
                          const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                          float& t, float& u, float& v);

#endif
