#pragma once

#include "sphere_collider.hpp"
#include "collision.hpp"
#include "../lve_game_object.hpp"

#include <glm/glm.hpp>

namespace lve
{
    class BoxCollider
    {
    public:
        BoxCollider(glm::vec3 position, glm::vec4 axis1, glm::vec4 axis2, glm::vec4 axis3);
        BoxCollider(glm::vec3 position, glm::vec3 axis1, glm::vec3 axis2, glm::vec3 axis3);

        bool isColliding(const SphereCollider& other);
        bool getImpulse(SphereCollider& other, Collision& collision);

        glm::vec3 position{0.0f};
    private:
        bool isSeparated(glm::vec3 distance, glm::vec3 normal, float width1, float width2);

        glm::vec4 normals[3];
        glm::vec3 axes[3];
    };
}