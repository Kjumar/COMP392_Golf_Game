#include "collision_manager.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace lve
{
    CollisionManager::CollisionManager()
    {

    }

    glm::vec3 CollisionManager::readVec3(const std::string& line)
    {
        std::istringstream l{ line };
        std::string word;
        std::getline(l, word, '/');
        float x = std::stof(word);
        std::getline(l, word, '/');
        float y = std::stof(word);
        std::getline(l, word, '/');
        float z = std::stof(word);

        return glm::vec3{x, y, z};
    }

    std::vector<BoxCollider> CollisionManager::readCollidersFromFile(const std::string& filename)
    {
        std::vector<BoxCollider> colliders;

        std::ifstream colliderFile{ ENGINE_DIR + filename };
        std::string line;

        if (!colliderFile.is_open())
        {
            throw std::runtime_error("failed to open file: " + (ENGINE_DIR + filename));
        }

        try
        {
            while (std::getline(colliderFile, line))
            {
                std::istringstream l{ line };
                std::string word;
                std::getline(l, word, ' ');
                float x = std::stof(word);
                std::getline(l, word, ' ');
                float y = std::stof(word);
                std::getline(l, word, ' ');
                float z = std::stof(word);

                std::getline(l, word, ' ');
                glm::vec3 width = readVec3(word);
                std::getline(l, word, ' ');
                glm::vec3 height = readVec3(word);
                std::getline(l, word, ' ');
                glm::vec3 depth = readVec3(word);

                // std::cout << x << ", " << y << ", " << z << '\n';
                // std::cout << width.x << '/' << width.y << '/' << width.z << '\n';
                // std::cout << height.x << '/' << height.y << '/' << height.z << '\n';
                // std::cout << depth.x << '/' << depth.y << '/' << depth.z << '\n';

                BoxCollider newCollider{glm::vec3{x, y, z}, width, height, depth };
                colliders.push_back(newCollider);
            }
        }
        catch(const std::exception& e)
        {
            std::cout << "error reading collider file. Make sure its in the correct format" << '\n';
        }

        colliderFile.close();

        return colliders;
    }
}