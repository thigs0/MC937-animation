#ifndef OBJ_LOADER_HPP
#define OBJ_LOADER_HPP

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

struct Face {
    std::vector<unsigned int> vertex_indices;
    std::vector<unsigned int> normal_indices;
    std::string material_name;
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

bool loadOBJ(const std::string& path,
             std::vector<glm::vec3>& out_vertices,
             std::vector<glm::vec3>& out_normals,
             std::vector<Face>& out_faces);

             
bool loadOBJWithMTL(const std::string& objPath,
                    std::vector<glm::vec3>& out_vertices,
                    std::vector<glm::vec3>& out_normals,
                    std::vector<Face>& out_faces,
                    std::map<std::string, Material>& out_materials);

#endif
