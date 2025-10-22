#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "util.hpp"

namespace ke
{
	namespace prim
	{
		struct Rect
		{
		public:
			Rect() = default;
			Rect(glm::vec2 _xy, glm::vec2 _extent);
			~Rect();

			glm::vec2 xy;
			glm::vec2 extent;

			void Draw(VkCommandBuffer commandBuffer) const;
		private:
			std::vector<ke::str::Vertex> mVertices;
			std::vector<uint16_t> mIndices;

			VkBuffer mVertexBuffer;
			VkDeviceMemory mVertexBufferMemory;
			VkBuffer mIndexBuffer;
			VkDeviceMemory mIndexBufferMemory;

			void createVulkanBuffers();
		};
	}
}