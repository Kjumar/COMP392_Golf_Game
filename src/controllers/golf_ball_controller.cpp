#include "golf_ball_controller.hpp"

namespace lve
{
    GolfBallController::GolfBallController(LveGameObject& gameObject, float radius)
        : gameObject{ gameObject }, collider{gameObject.transform.translation, radius * gameObject.transform.scale.x}
    {
    }

    void GolfBallController::update(GLFWwindow* window, float dt)
    {
        if (isAttached)
        {
            glm::vec3 rotate{ 0 };
            if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
            if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;

            if (!moving)
            {
                if (aiming)
                {
                    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) power += 0.2f;
                    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) power -= 0.2f;
                    power = glm::clamp(power, 0.0f, maxPower);

                    if (glfwGetKey(window, keys.launch) == GLFW_RELEASE)
                    {
                        aiming = false;
                        float yaw = aimRotation.y;
                        velocity = { sin(yaw), 0.0f, cos(yaw) };
                        velocity = velocity * power;
                        moving = true;
                        power = 0.0f;
                    }
                }
                else
                {
                    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
                    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;

                    if (glfwGetKey(window, keys.launch) == GLFW_PRESS)
                    {
                        aiming = true;
                    }
                }
                if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
                    aimRotation += lookSpeed * dt * glm::normalize(rotate);
                }
                aimRotation.x = glm::clamp(aimRotation.x, -1.5f, 1.5f);
                aimRotation.y = glm::mod(aimRotation.y, glm::two_pi<float>());
                return;
            }
            if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
            if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;
            if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
                aimRotation += lookSpeed * dt * glm::normalize(rotate);
            }
            aimRotation.x = glm::clamp(aimRotation.x, -1.5f, 1.5f);
            aimRotation.y = glm::mod(aimRotation.y, glm::two_pi<float>());
        }

        if (glm::dot(velocity, velocity) < 0.1)
        {
            moving = false;
            return;
        }

        velocity.y += gravity * dt;
        glm::vec3 movement = {velocity.x, velocity.y, velocity.z};

        gameObject.transform.translation += movement * dt;
    }

    SphereCollider& GolfBallController::getCollider()
    {
        collider.position = gameObject.transform.translation;
        return collider;
    }

    void GolfBallController::onCollision(const Collision& collision)
    {
        gameObject.transform.translation += (collision.normal * collision.depth);
        collider.position = gameObject.transform.translation;

        float impulse;
        // the collision struct could use a value to 'dampen' the object's energy on impact
        // for now, since all floors should be soft, I'm just doing it manually
        if (collision.normal.y <= -0.9f)
        {
            impulse = glm::dot(velocity, collision.normal) * 1.25f;
        }
        else
        {
            impulse = glm::dot(velocity, collision.normal) * 2;
        }
        velocity = (velocity - (collision.normal * impulse)) * friction;
    }

    bool GolfBallController::showReticle()
    {
        return aiming;
    }

    float GolfBallController::getPowerRatio()
    {
        return power / maxPower;
    }
}