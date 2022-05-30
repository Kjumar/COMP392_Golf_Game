#pragma once

#include "sphere_collider.hpp"
#include "box_collider.hpp"

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

    private:
        glm::vec3 readVec3(const std::string& line);
    };
}