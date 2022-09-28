#pragma once

#include "collision.hpp"
#include "quad_tree.hpp"
#include <glm/glm.hpp>

namespace lve
{
    class ICollider
    {
    public:
        virtual bool CollidesWith(ICollider& other) = 0;
        virtual float GetLengthAlongNormal(glm::vec3 normal) const = 0;
        virtual AABB GetAABB() = 0;

        virtual bool GetImpulse(ICollider* other, Collision& collision) = 0;

        glm::vec3 position;
    };
}