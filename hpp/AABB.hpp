#include <iostream>
#include <vector>
#include <array>
#include<glm/glm.hpp>
#include <memory>
#include <algorithm>

// Axis-Aligned Bounding Box (AABB) structure
struct AABB
{
    glm::vec3 min_corner{}; // Minimum corner of the bounding box
    glm::vec3 max_corner{}; // Maximum corner of the bounding box

    AABB() {}

    // Constructor that initializes the bounding box with given min and max
    AABB(const glm::vec3& min, const glm::vec3& max):
        max_corner{max},
        min_corner{min}
    {}

    // Checks if is valid 
    bool contains(const glm::vec3& p) const {
        return (p.x >= min_corner.x && p.x <= max_corner.x
              && p.y >= min_corner.y && p.y <= max_corner.y
              && p.z >= min_corner.z && p.z <= max_corner.z);
    }

    // Returns the axis (0=x, 1=y, 2=z) where the box is largest
    unsigned short getLargestAxis() const
    {
        glm::vec3 size = max_corner - min_corner;

        if (size.x >= size.y && size.x >= size.z)
        {
            return 0;
        }
        else if (size.y >= size.z)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    // Allows printing the AABB to the output
    friend std::ostream& operator<<(std::ostream& os, const AABB& aabb) {
        os << "AABB Corners:\n"
           << "  Min: (" << aabb.min_corner.x << ", "
           << aabb.min_corner.y << ", "
           << aabb.min_corner.z << ")\n"
           << "  Max: (" << aabb.max_corner.x << ", "
           << aabb.max_corner.y << ", "
           << aabb.max_corner.z << ")";
        return os;
    }

    // Splits the AABB into two along a specified axis
    std::pair<AABB, AABB> split(unsigned axis) const;
};

// Splits the AABB into two halves along the given axis
std::pair<AABB, AABB> AABB::split(unsigned axis) const
{
    float mid_point = (min_corner[axis] + max_corner[axis]) * 0.5f;
    std::cout << mid_point << "mid \n";

    glm::vec3 first_max{max_corner};
    first_max[axis] = mid_point;

    glm::vec3 second_min{min_corner};
    second_min[axis] = mid_point;

    return {
        AABB(min_corner, first_max),
        AABB(second_min, max_corner)
    };
}

// Structure representing a 3D mesh
struct Mesh
{
    using coordinate_t = std::vector<glm::vec3>; // List of vertex positions
    using triangles_t = std::vector<std::array<unsigned, 3>>; // List of triangle indices

    AABB aabb; // Bounding box for the mesh
    coordinate_t coordinates; // Vertex positions
    triangles_t triangles; // Triangles made of 3 vertex indices

    // Constructor that takes coordinates and triangle data
    Mesh(const coordinate_t& coordinates,
         const triangles_t& triangles):
        coordinates{coordinates},
        triangles{triangles}
    {
        updateAABB(); // Automatically compute the AABB
    }

    void updateAABB(); // Updates the mesh's bounding box

    // Sorts triangles based on their centroid's position along a given axis
    void sortTrianglesByAxis(unsigned axis)
    {
        auto comparator = [&](const std::array<unsigned, 3>& a, const std::array<unsigned, 3>& b)
        {
            glm::vec3 centroid_a = (coordinates[a[0]] + coordinates[a[1]] + coordinates[a[2]]) / 3.0f;
            glm::vec3 centroid_b = (coordinates[b[0]] + coordinates[b[1]] + coordinates[b[2]]) / 3.0f;
            return centroid_a[axis] < centroid_b[axis];
        };

        std::sort(triangles.begin(), triangles.end(), comparator);
    }

    // Splits the mesh's triangle list into two halves
    std::pair<Mesh, Mesh> splitMesh();
};

// Splits the mesh into two submeshes based on the triangle list
std::pair<Mesh, Mesh> Mesh::splitMesh()
{
    const size_t split_index = triangles.size() / 2;

    triangles_t first_t, second_t;

    // Divide triangles into two halves
    first_t.assign(triangles.begin(), triangles.begin() + split_index);
    second_t.assign(triangles.begin() + split_index, triangles.end());

    return {{coordinates, first_t}, {coordinates, second_t}};
}

// Computes the AABB that encloses all the mesh's vertices
void Mesh::updateAABB()
{
    glm::vec3 min{coordinates[0]};
    glm::vec3 max{coordinates[0]};

    for(const auto& coord: coordinates)
    {
        min = glm::min(min, coord);
        max = glm::max(max, coord);
    }

    aabb = {min, max};
}

// Node in the AABB tree
struct AABBNode
{
    Mesh mesh;
    bool leaf{false}; // True if this node is a leaf (only one triangle)

    std::unique_ptr<AABBNode> left_child{nullptr};
    std::unique_ptr<AABBNode> right_child{nullptr};

    AABBNode(const Mesh& mesh):mesh{mesh}{};

    bool isLeaf() const {return leaf;}
    void makeLeaf() { leaf = true;}
};

// AABB Tree structure
struct AABBTree
{
    std::unique_ptr<AABBNode> root{nullptr}; // Root node of the tree

    AABBTree(const Mesh& mesh); // Constructor builds root node
    void build();               // Starts recursive construction
    void print(std::unique_ptr<AABBNode>& node) const; // Prints tree content

private:
    void build(std::unique_ptr<AABBNode>& node); // Recursive builder
};
