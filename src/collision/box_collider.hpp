#pragma once

#include "sphere_collider.hpp"
#include "collision.hpp"
#include "../lve_game_object.hpp"
#include "quad_tree.hpp"
#include "icollider.hpp"

#include <glm/glm.hpp>

namespace lve
{
    class BoxCollider : public ICollider
    {
    public:
        BoxCollider(glm::vec3 position, glm::vec4 axis1, glm::vec4 axis2, glm::vec4 axis3);
        BoxCollider(glm::vec3 position, glm::vec3 axis1, glm::vec3 axis2, glm::vec3 axis3);
        bool GetImpulse(ICollider* other, Collision& collision);

        bool CollidesWith(ICollider& other);
        float GetLengthAlongNormal(glm::vec3 normal) const;
        AABB GetAABB();

        LveModel* GetWireFrame(LveDevice& device, glm::vec3 color);

    private:
        bool isSeparated(glm::vec3 distance, glm::vec3 normal, float width1, float width2);

        glm::vec4 normals[3];
        glm::vec3 axes[3];
    };
}