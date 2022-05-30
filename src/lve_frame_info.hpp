#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace lve
{
	#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

    struct GlobalUbo {
        glm::mat4 projection{ 1.0f };
        glm::mat4 view{ 1.0f };
		glm::mat4 inverseView{ 1.0f };
        glm::vec4 ambientLightColor{ 0.8f, 0.8f, 1.0f, 0.5f };
		glm::vec4 direcitonalLight{ 0.0f, -1.0f, 0.2f, 0.8f };
        PointLight pointLights[MAX_LIGHTS];
		int numLights;
    };

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LveCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		LveGameObject::Map& gameObjects;
	};
} // namespace lve