#pragma once

#include "icollider.hpp"
#include <glm/glm.hpp>

namespace lve
{
    class SphereCollider : public ICollider
    {
    public:
        SphereCollider(glm::vec3 position, float radius);

        bool CollidesWith(ICollider& other);
        float GetLengthAlongNormal(glm::vec3 normal) const;
        AABB GetAABB();

        bool GetImpulse(ICollider* other, Collision& collision);
        
        float radius;
    };
}