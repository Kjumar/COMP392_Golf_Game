#pragma once

#include <glm/glm.hpp>

namespace lve
{
    struct Collision
    {
        glm::vec3 normal{};
        float depth{};
    };
}