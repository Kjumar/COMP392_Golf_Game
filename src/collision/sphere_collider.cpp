#include "sphere_collider.hpp"

namespace lve
{
    SphereCollider::SphereCollider(glm::vec3 position, float radius)
        : radius{ radius }
    {
        this->position = position;
    }

    bool SphereCollider::CollidesWith(ICollider& other)
    {
        glm::vec3 d = position - other.position;
        float otherLength = other.GetLengthAlongNormal(d);
        float lengthSquared = glm::dot(d, d);

        return lengthSquared < otherLength + radius;
    }

    float SphereCollider::GetLengthAlongNormal(glm::vec3 normal) const
    {
        return radius;
    }

    AABB SphereCollider::GetAABB()
    {
        return {0, glm::vec2(position.x - radius, position.z - radius), glm::vec2(position.x + radius, position.z + radius)};
    }

    bool SphereCollider::GetImpulse(ICollider* other, Collision& collision)
    {
        return false;
    }
}