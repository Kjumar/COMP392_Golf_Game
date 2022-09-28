
#include "quad_tree.hpp"

namespace lve
{
    const int MAXCAPACITY = 4;

    QuadTree::QuadTree()
    {

    }

    void QuadTree::Insert(const AABB rect, const AABB bounds)
    {
        if (children[0] != nullptr)
        {
            // if this node has laready been split (meaning its over capacity)
            int index = GetChildIndex(rect, bounds);
            if (index != -1)
            {
                children[index]->Insert(rect, GetChildBounds(bounds, index));
                return;
            }
            boxes.push_back(rect);
            return;
        }
        boxes.push_back(rect);
        if (boxes.size() >= MAXCAPACITY)
        {
            Split();
            int i = 0;
            while(i < boxes.size())
            {
                int index = GetChildIndex(boxes[i], bounds);
                if (index == -1)
                {
                    i++;
                }
                else
                {
                    children[index]->Insert(boxes[i], GetChildBounds(bounds, index));
                    boxes.erase(boxes.begin() + i);
                }
            }
        }
    }

    void QuadTree::Split()
    {
        children[0] = new QuadTree();
        children[1] = new QuadTree();
        children[2] = new QuadTree();
        children[3] = new QuadTree();
    }

    AABB QuadTree::GetChildBounds(AABB bounds, int index)
    {
        glm::vec2 halfSize = (bounds.max - bounds.min) / glm::vec2(2, 2);

        if (index == 0)
        {
            // return topleft
            return {0, bounds.min, bounds.min + halfSize};
        }
        else if (index == 1)
        {
            // return topright
            return {0, bounds.min + glm::vec2(halfSize.x, 0), glm::vec2(bounds.max.x, bounds.min.y + halfSize.y)};
        }
        else if (index == 2)
        {
            // return bottomright
            return {0, bounds.min + halfSize, bounds.max};
        }
        return {0, bounds.min + glm::vec2(0, halfSize.y), glm::vec2(bounds.min.x + halfSize.x, bounds.max.y)};
    }

    int QuadTree::GetChildIndex(const AABB rect, const AABB bounds)
    {
        glm::vec2 midpoint = (bounds.max + bounds.min) / glm::vec2(2, 2);

        if (rect.max.y < midpoint.y)
        {
            if (rect.max.x < midpoint.x)
            {
                return 0;
            }
            else if (rect.min.x > midpoint.x)
            {
                return 1;
            }
        }
        else if (rect.min.y > midpoint.y)
        {
            if (rect.min.x > midpoint.x)
            {
                return 2;
            }
            if (rect.max.x < midpoint.x)
            {
                return 3;
            }
        }
        return -1;
    }

    void QuadTree::Retrieve(std::vector<int>& colliders, const AABB rect, const AABB bounds)
    {
        glm::vec2 midpoint = (bounds.max + bounds.min) / glm::vec2(2, 2);

        for (AABB box : boxes)
        {
            colliders.push_back(box.colliderIndex);
        }

        if (children[0] != nullptr)
        {
            if (rect.min.y < midpoint.y)
            {
                if (rect.min.x < midpoint.x)
                {
                    children[0]->Retrieve(colliders, rect, GetChildBounds(bounds, 0));
                }
                if (rect.max.x > midpoint.x)
                {
                    children[1]->Retrieve(colliders, rect, GetChildBounds(bounds, 1));
                }
            }
            if (rect.max.y > midpoint.y)
            {
                if (rect.min.x < midpoint.x)
                {
                    children[3]->Retrieve(colliders, rect, GetChildBounds(bounds, 3));
                }
                if (rect.max.x > midpoint.x)
                {
                    children[2]->Retrieve(colliders, rect, GetChildBounds(bounds, 2));
                }
            }
        }
    }

    QuadTree::~QuadTree()
    {
        if (children[0] != nullptr)
        {
            delete(children[0]);
            delete(children[1]);
            delete(children[2]);
            delete(children[3]);
            children[0] = nullptr;
            children[1] = nullptr;
            children[2] = nullptr;
            children[3] = nullptr;
        }
        boxes.clear();
    }
}