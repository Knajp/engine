#pragma once
#include <glm/glm.hpp>
#include "Rect.hpp"
#include <functional>

namespace ke
{
	namespace ui
	{
		class GUIelement
		{
		public:
			GUIelement() = default;
			GUIelement(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color);

			void Draw(VkCommandBuffer buffer) const;
		protected:
			ke::prim::Rect mRectangle;
		};

		class Button : public GUIelement
		{
		public:
			Button() = default;
			Button(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color);

			bool isHovering(glm::vec2 cursorPositon) const;
			std::function<void()> onClick;

		};
	}
}