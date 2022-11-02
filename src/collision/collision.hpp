#pragma once

#include <glm/glm.hpp>
#include "../lve_game_object.hpp"

namespace lve
{
    struct Collision
    {
        glm::vec3 normal{};
        float depth{};
        LveGameObject* gameObject;
    };
}