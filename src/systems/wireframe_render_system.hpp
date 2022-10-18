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
	class WireframeRenderSystem {
	public:

		WireframeRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~WireframeRenderSystem();

		WireframeRenderSystem(const WireframeRenderSystem&) = delete;
		WireframeRenderSystem& operator=(const WireframeRenderSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo, std::vector<LveModel*> models);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		LveDevice& lveDevice;

		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}  // namespace lve