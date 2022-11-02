#pragma once

#include "../lve_game_object.hpp"
#include "../lve_window.hpp"
#include "../collision/collision.hpp"
#include "../collision/sphere_collider.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lve
{
    class GolfBallController
    {
    public:
        struct KeyMappings {
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
            int launch = GLFW_KEY_SPACE;
		};

        GolfBallController(LveGameObject& gameObject, float radius);

        void update(GLFWwindow* window, float dt);

        SphereCollider& getCollider();
        void onCollision(const Collision& collision);
        static void ForwardOnCollision(void* context, Collision collision)
        {
            static_cast<GolfBallController*>(context)->onCollision(collision);
        }

        bool showReticle();
        float getPowerRatio();

        void resetBall();
        void resetBall(glm::vec3 position);
        void nextHole(glm::vec3 position);
        bool isMoving();
        
        LveGameObject& gameObject;
        // horizontal rotation for where the player is aiming
        glm::vec3 aimRotation{-1.0f, -1.5f, 0.0f};

        bool isAttached = true;
    private:
        KeyMappings keys{};
		glm::vec3 velocity{ 0.0f };
        float power{ 0.0f };
        float maxPower{ 10.0f };
		float lookSpeed{ 1.5f };
        float gravity{ 14.7f };
        float friction{ 0.6f };
        float drag{ 0.1f };

        bool moving = false;
        bool aiming = false;
        bool bIsGrounded = false;

        std::vector<std::string> courseIds = {"c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9"};
        int currentCourse = 0;
        glm::vec3 previousPos{0.0f};

        SphereCollider collider;
    };
}