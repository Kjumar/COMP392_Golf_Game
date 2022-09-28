#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include <iostream>

namespace lve
{
    // axis aligned bounding box with precomputed mins and maxs
    // these are only 2-dimensional since the golf game is mostly flat and subdividing the tree
    // over the height axis (ingame y-axis) would be a waste
    struct AABB
    {
        uint32_t colliderIndex;
        glm::vec2 min;
        glm::vec2 max;
    };

    class QuadTree
    {
    public:
        QuadTree();
        ~QuadTree();
        void Insert(const AABB rect, const AABB bounds);
        void Retrieve(std::vector<int>& colliders, const AABB rect, const AABB bounds);

        // for debugging the tree
        void Display(std::string path)
        {
            std::cout << path << "\n";

            for (AABB aabb : boxes)
            {
                std::cout << aabb.colliderIndex << "\n";
            }

            if (children[0] != nullptr)
            {
                children[0]->Display(path + "->0");
                children[1]->Display(path + "->1");
                children[2]->Display(path + "->2");
                children[3]->Display(path + "->3");
            }
        }
        
    private:
        void Split();
        int GetChildIndex(const AABB rect, const AABB bounds);
        AABB GetChildBounds(AABB bounds, int index);

        QuadTree* children[4] = {nullptr, nullptr, nullptr, nullptr};
        std::vector<AABB> boxes;
    };
}