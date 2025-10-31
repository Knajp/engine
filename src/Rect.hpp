#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "util.hpp"
#include "texture.hpp"

namespace ke
{
	namespace prim
	{
		struct Rect
		{
		public:
			Rect() = default;
			Rect(glm::vec2 _xy, glm::vec2 _extent, glm::vec3 _color);
			~Rect();

			Rect(const Rect& other);
			void operator=(const Rect& other);

			glm::vec2 xy;
			glm::vec2 extent;
			glm::vec3 color;

			void Draw(VkCommandBuffer commandBuffer) const;
			bool intersects(glm::vec2 point) const;

			void setTexture(ke::Texture text);
		private:
			std::vector<ke::str::Vertex> mVertices;
			std::vector<uint16_t> mIndices;

			VkBuffer mVertexBuffer = VK_NULL_HANDLE;
			VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;
			VkBuffer mIndexBuffer = VK_NULL_HANDLE;
			VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;

			ke::Texture mTexture;
			int useTexture = 0;

			void createVulkanBuffers();
		};
	}
}