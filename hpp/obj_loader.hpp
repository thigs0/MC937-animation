#ifndef OBJ_LOADER_HPP
#define OBJ_LOADER_HPP

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "hpp/physics.hpp" 
#include "hpp/materials.hpp"

bool loadOBJ(const std::string& objPath, PhysicalObject* object);

#endif
