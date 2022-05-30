#pragma once

#include <glm/glm.hpp>

namespace lve
{
    class SphereCollider
    {
    public:
        SphereCollider(glm::vec3 position, float radius);

        bool isColliding(const SphereCollider& other);
        
        glm::vec3 position;
        float radius;
    };
}