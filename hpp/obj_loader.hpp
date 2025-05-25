#ifndef OBJ_LOADER_HPP
#define OBJ_LOADER_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Face {
    std::vector<unsigned int> vertex_indices;
    std::vector<unsigned int> normal_indices;
};

bool loadSimpleOBJ(const std::string& path,
             std::vector<glm::vec3>& out_vertices,
             std::vector<glm::vec3>& out_normals,
             std::vector<Face>& out_faces);

#endif

