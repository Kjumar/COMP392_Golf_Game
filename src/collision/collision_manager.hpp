#pragma once

#include "sphere_collider.hpp"
#include "box_collider.hpp"
#include "quad_tree.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace lve
{
    class CollisionManager
    {
    public:
        CollisionManager();

        std::vector<BoxCollider> readCollidersFromFile(const std::string& filename);

        void InsertStaticCollider(ICollider* collider);
        
        void buildStaticTree();

        void GetCollisions(ICollider& collider, void (*OnCollision)(void*, Collision), void* context);

    private:
        glm::vec3 readVec3(const std::string& line);

        bool rebuildTree = true;

        QuadTree staticTree;

        std::vector<ICollider*> staticColliders;
    };
}