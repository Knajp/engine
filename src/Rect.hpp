#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
namespace ke
{
	namespace prim
	{
		struct Rect
		{
		public:
			Rect() = default;
			Rect(glm::vec2 _xy, glm::vec2 _extent);

			glm::vec2 xy;
			glm::vec2 extent;

		private:
			void createVulkanBuffers();
		};
	}
}