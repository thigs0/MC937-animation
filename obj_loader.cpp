#include "obj_loader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include "hpp/physics.hpp" 

// Função para calcular normal de face
glm::vec3 computeFaceNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
    return glm::normalize(glm::cross(v1 - v0, v2 - v0));
}

// Função para calcular normais por vértice a partir de faces e vértices
void calculateNormalsFromVertices(const std::vector<glm::vec3>& vertices,
                                  const std::vector<Face>& faces,
                                  std::vector<glm::vec3>& normals) {
    normals.clear();
    normals.resize(vertices.size(), glm::vec3(0.0f));

    for (const auto& face : faces) {
        if (face.vertex_indices.size() < 3) continue;
        glm::vec3 v0 = vertices[face.vertex_indices[0]];
        glm::vec3 v1 = vertices[face.vertex_indices[1]];
        glm::vec3 v2 = vertices[face.vertex_indices[2]];

        glm::vec3 faceNormal = computeFaceNormal(v0, v1, v2);

        for (auto vi : face.vertex_indices) {
            normals[vi] += faceNormal;
        }
    }

    for (auto& n : normals) {
        if (glm::length(n) > 0.0f)
            n = glm::normalize(n);
        else
            n = glm::vec3(0.0f, 1.0f, 0.0f); // normal padrão caso zero
    }
}

bool loadMTL(const std::string& mtlPath, std::map<std::string, Material>& materials) {
    std::ifstream file(mtlPath);
    if (!file.is_open()) return false;

    std::string line, currentMat;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;
        if (prefix == "newmtl") {
            ss >> currentMat;
            materials[currentMat] = Material();
        } else if (prefix == "Ka") {
            ss >> materials[currentMat].ambient.r >> materials[currentMat].ambient.g >> materials[currentMat].ambient.b;
        } else if (prefix == "Kd") {
            ss >> materials[currentMat].diffuse.r >> materials[currentMat].diffuse.g >> materials[currentMat].diffuse.b;
        } else if (prefix == "Ks") {
            ss >> materials[currentMat].specular.r >> materials[currentMat].specular.g >> materials[currentMat].specular.b;
        }
    }
    return true;
}

bool loadOBJ_aux(const std::string& objPath,
                    std::vector<glm::vec3>& out_vertices,
                    std::vector<glm::vec3>& out_normals,
                    std::vector<Face>& out_faces,
                    std::map<std::string, Material>& out_materials)
{
    std::ifstream file(objPath);
    if (!file.is_open()) return false;

    std::string mtlFile;
    std::vector<glm::vec3> temp_vertices, temp_normals;
    std::string currentMaterial;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "mtllib") {
            ss >> mtlFile;
            loadMTL(mtlFile, out_materials);
        } else if (prefix == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        } else if (prefix == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        } else if (prefix == "usemtl") {
            ss >> currentMaterial;
        } else if (prefix == "f") {
            Face face;
            face.material_name = currentMaterial;

            std::string vertex_info;
            while (ss >> vertex_info) {
                size_t pos1 = vertex_info.find('/');
                size_t pos2 = vertex_info.find_last_of('/');

                unsigned int vi = std::stoi(vertex_info.substr(0, pos1)) - 1;

                unsigned int ni = 0;
                if (pos2 != std::string::npos && pos2 > pos1)
                    ni = std::stoi(vertex_info.substr(pos2 + 1)) - 1;
                else
                    ni = 0; // default

                face.vertex_indices.push_back(vi);
                face.normal_indices.push_back(ni);
            }
            out_faces.push_back(face);
        }
    }

    out_vertices = temp_vertices;
    out_normals = temp_normals;

    // Se não tem normais no OBJ, calcular
    if (out_normals.empty()) {
        calculateNormalsFromVertices(out_vertices, out_faces, out_normals);
        // Atualiza os índices normais para serem iguais aos índices dos vértices
        for (auto& face : out_faces) {
            face.normal_indices.clear();
            for (auto vi : face.vertex_indices) {
                face.normal_indices.push_back(vi);
            }
        }
    }

    return true;
}

bool loadOBJ(const std::string& objPath, PhysicalObject* object)
{
    std::vector<glm::vec3> vertices, normals;
    std::vector<Face> faces;
    std::map<std::string, Material> materials;

    bool success = loadOBJ_aux(objPath, vertices, normals, faces, materials);
    if (!success) {
        std::cerr << "Erro ao carregar OBJ: " << objPath << std::endl;
        return false;
    }

    object->vertices = vertices;
    object->normals = normals;
    object->faces = faces;
    object->materials = materials;

    return true;
}

