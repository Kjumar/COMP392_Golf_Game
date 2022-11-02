#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_frame_info.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
	class GolfturfRenderSystem {
	public:

		GolfturfRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~GolfturfRenderSystem();

		GolfturfRenderSystem(const GolfturfRenderSystem&) = delete;
		GolfturfRenderSystem& operator=(const GolfturfRenderSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo, int materialId, float time);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		LveDevice& lveDevice;

		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}  // namespace lve