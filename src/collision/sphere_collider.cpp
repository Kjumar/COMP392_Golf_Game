#include "sphere_collider.hpp"

namespace lve
{
    SphereCollider::SphereCollider(glm::vec3 position, float radius)
        : position{ position }, radius{ radius }
    {

    }

    bool SphereCollider::isColliding(const SphereCollider& other)
    {
        glm::vec3 d = position - other.position;
        float distance = glm::dot(d, d);
        float radii = radius + other.radius;

        return (distance > (radii * radii));
    }
}