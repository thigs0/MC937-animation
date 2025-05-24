#include <iostream>
#include <vector>
#include <array>
#include<glm/glm.hpp>
#include <memory>
#include <hpp/AABB.hpp>


// Prints the first triangle of each leaf node in the tree
void AABBTree::print(std::unique_ptr<AABBNode>& node) const
{
    if(node==nullptr) {return;}
    if(!node->isLeaf())
    {
        print(node->left_child);
        print(node->right_child);
    }

    auto triangle = node->mesh.triangles[0];
    for(auto t:triangle)
    {
        std::cout << node->mesh.coordinates[t][0] << ", ";
        std::cout << node->mesh.coordinates[t][1] << ", ";
        std::cout << node->mesh.coordinates[t][2] << "\n";
    }
    std::cout << '\n';
}

// Creates the root node of the AABB tree
AABBTree::AABBTree(const Mesh& mesh)
{
    root = std::make_unique<AABBNode>(mesh);
}

// Builds the AABB tree and prints its structure
void AABBTree::build()
{
    build(root);
    print(root);
    std::cout << "[ OK ] Build AABB tree\n";
}

// Recursively builds the AABB tree
void AABBTree::build(std::unique_ptr<AABBNode>& node)
{
    if(node == nullptr) { return; }
    if(node->isLeaf() ) { return; }

    // Stop recursion if there is only one triangle
    if(node->mesh.triangles.size() == 1)
    {
        node->makeLeaf();
        return;
    }

    // Determine the longest axis of the current AABB
    unsigned short largests_axis = node->mesh.aabb.getLargestAxis();

    // Sort triangles along that axis
    node->mesh.sortTrianglesByAxis(largests_axis);

    // Split the mesh into two
    auto sub_meshes = node->mesh.splitMesh();

    // Create child nodes
    node->left_child = std::make_unique<AABBNode>(sub_meshes.first);
    node->right_child = std::make_unique<AABBNode>(sub_meshes.second);

    // Recurse
    build(node->left_child);
    build(node->right_child);
}
