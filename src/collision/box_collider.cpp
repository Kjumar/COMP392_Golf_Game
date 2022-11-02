#include "box_collider.hpp"
#include <cstdlib>
#include <algorithm>

#include <iostream>
#include <vector>

namespace lve
{
    BoxCollider::BoxCollider(glm::vec3 position, glm::vec4 axis1, glm::vec4 axis2, glm::vec4 axis3)
    {
        this->position = position;

        normals[0] = axis1;
        normals[1] = axis2;
        normals[2] = axis3;

        axes[0] = glm::vec3{axis1.x, axis1.y, axis1.z} * axis1.w;
        axes[1] = glm::vec3{axis2.x, axis2.y, axis2.z} * axis2.w;
        axes[2] = glm::vec3{axis3.x, axis3.y, axis3.z} * axis3.w;
    }

    BoxCollider::BoxCollider(glm::vec3 position, glm::vec3 axis1, glm::vec3 axis2, glm::vec3 axis3)
    {
        this->position = position;

        axes[0] = axis1;
        axes[1] = axis2;
        axes[2] = axis3;

        float mag1 = glm::length(axis1);
        float mag2 = glm::length(axis2);
        float mag3 = glm::length(axis3);

        if (mag1 > 0) { normals[0] = glm::vec4{axis1.x / mag1, axis1.y / mag1, axis1.z / mag1, mag1}; }
        if (mag2 > 0) { normals[1] = glm::vec4{axis2.x / mag2, axis2.y / mag2, axis2.z / mag2, mag2}; }
        if (mag3 > 0) { normals[2] = glm::vec4{axis3.x / mag3, axis3.y / mag3, axis3.z / mag3, mag3}; }

        //std::cout << mag1 << " / " << mag2 << " / " << mag3 << '\n';
    }

    // returns true if there is separation between two objects with given width along the axis defined by the normal
    bool BoxCollider::isSeparated(glm::vec3 distance, glm::vec3 normal, float width1, float width2)
    {
        float d = std::abs(glm::dot(distance, normal));
        if (d > width1 + width2)
        {
            return true;
        }
        return false;
    }

    // bool BoxCollider::isColliding(const SphereCollider& other)
    // {
    //     glm::vec3 d = other.position - position;

    //     for (glm::vec4 axis : normals)
    //     {
    //         glm::vec3 normal{ axis.x, axis.y, axis.z };
    //         if (isSeparated(d, normal, axis.w, other.radius))
    //         {
    //             return false;
    //         }
    //     }
        
    //     float c1 = glm::abs(glm::dot(axes[0], d));
    //     float c2 = glm::abs(glm::dot(axes[1], d));
    //     float c3 = glm::abs(glm::dot(axes[2], d));
    //     float v = glm::dot(d, d);
    //     if (v > c1 + c2 + c3 + other.radius)
    //     {
    //         return false;
    //     }

    //     return true;
    // }

    bool BoxCollider::GetImpulse(ICollider* other, Collision& collision)
    {
        /*
            a, b, c | x   [ax + by + cz] = l1
            d, e, f | y = [dx + ey + fz] = l2
            g, h, i | z   [gx + hy + iz] = l3
            */
        glm::vec3 delta = other->position - position;

        float component1 = glm::dot(delta, { normals[0].x, normals[0].y, normals[0].z });
        float component2 = glm::dot(delta, { normals[1].x, normals[1].y, normals[1].z });
        float component3 = glm::dot(delta, { normals[2].x, normals[2].y, normals[2].z });

        glm::vec3 closest = { component1, component2, component3 };

        closest.x = std::clamp(component1, -normals[0].w, normals[0].w);
        closest.y = std::clamp(component2, -normals[1].w, normals[1].w);
        closest.z = std::clamp(component3, -normals[2].w, normals[2].w);

        bool inside = false;

        if (closest == glm::vec3{ component1, component2, component3 })
        {
            inside = true;

            float absX = normals[0].w - std::abs(closest.x);
            float absY = normals[1].w - std::abs(closest.y);
            float absZ = normals[2].w - std::abs(closest.z);

            if (absX < absY && absX < absZ)
            {
                if (closest.x > 0)
                {
                    closest.x = normals[0].w;
                }
                else
                {
                    closest.x = -normals[0].w;
                }
            }
            else if (absY < absX && absY < absZ)
            {
                if (closest.y > 0)
                {
                    closest.y = normals[1].w;
                }
                else
                {
                    closest.y = -normals[1].w;
                }
            }
            else
            {
                if (closest.z > 0)
                {
                    closest.z = normals[2].w;
                }
                else
                {
                    closest.z = -normals[2].w;
                }
            }
        }

        glm::vec3 normal = glm::vec3{ component1, component2, component3 } - closest;
        glm::vec4 worldSpaceNorm = normals[0] * normal.x + normals[1] * normal.y + normals[2] * normal.z;
        normal = { worldSpaceNorm.x, worldSpaceNorm.y, worldSpaceNorm.z };
        float length = glm::dot(normal, normal);

        float otherLength = other->GetLengthAlongNormal(glm::normalize(normal));
        if (length > otherLength * otherLength && !inside)
        {
            return false;
        }
        
        length = std::sqrt(length);
        normal = normal / length;
        if (inside)
        {
            collision.normal = normal * -1.0f;
            collision.depth = otherLength - length;
        }
        else
        {
            collision.normal = normal;
            collision.depth = otherLength - length;
        }

        collision.gameObject = gameObject;

        return true;
    }

    bool BoxCollider::CollidesWith(ICollider& other)
    {
        glm::vec3 d = other.position - position;

        for (glm::vec4 axis : normals)
        {
            glm::vec3 normal{ axis.x, axis.y, axis.z };
            if (isSeparated(d, normal, axis.w, other.GetLengthAlongNormal(normal)))
            {
                return false;
            }
        }

        return true;
    }

    float BoxCollider::GetLengthAlongNormal(glm::vec3 normal) const
    {
        float c1 = glm::abs(glm::dot(axes[0], normal));
        float c2 = glm::abs(glm::dot(axes[1], normal));
        float c3 = glm::abs(glm::dot(axes[2], normal));

        return c1 + c2 + c3;
    }

    AABB BoxCollider::GetAABB()
    {
        float xLength = GetLengthAlongNormal(glm::vec3(1, 0, 0));
        float zLength = GetLengthAlongNormal(glm::vec3(0, 0, 1));

        AABB aabb = {0, glm::vec2(position.x - xLength, position.z - zLength),
            glm::vec2(position.x + xLength, position.z + zLength)};

        return aabb;
    }

    LveModel* BoxCollider::GetWireFrame(LveDevice& device, glm::vec3 color)
    {
        LveModel::Builder builder;

        builder.vertices.push_back({position + axes[0] + axes[1] + axes[2], color});
        builder.vertices.push_back({position + axes[0] + axes[1] - axes[2], color});
        builder.vertices.push_back({position + axes[0] - axes[1] + axes[2], color});
        builder.vertices.push_back({position + axes[0] - axes[1] - axes[2], color});
        builder.vertices.push_back({position - axes[0] + axes[1] + axes[2], color});
        builder.vertices.push_back({position - axes[0] + axes[1] - axes[2], color});
        builder.vertices.push_back({position - axes[0] - axes[1] + axes[2], color});
        builder.vertices.push_back({position - axes[0] - axes[1] - axes[2], color});

        builder.indices.push_back(0);
        builder.indices.push_back(1);
        builder.indices.push_back(2);

        builder.indices.push_back(5);
        builder.indices.push_back(6);
        builder.indices.push_back(7);

        builder.indices.push_back(0);
        builder.indices.push_back(4);
        builder.indices.push_back(5);

        builder.indices.push_back(2);
        builder.indices.push_back(3);
        builder.indices.push_back(7);

        builder.indices.push_back(2);
        builder.indices.push_back(4);
        builder.indices.push_back(6);

        builder.indices.push_back(1);
        builder.indices.push_back(3);
        builder.indices.push_back(5);

        return new LveModel(device, builder);
    }
}