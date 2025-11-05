#pragma once
#include <glm/glm.hpp>
#include "Rect.hpp"
#include <functional>
#include <algorithm>
#include "logger.hpp"

namespace ke
{
	namespace ui
	{
		inline ke::Logger logger = ke::Logger("UI logger", spdlog::level::debug);
		class GUIelement : public std::enable_shared_from_this<GUIelement>
		{
		public:
			GUIelement() = default;
			GUIelement(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color);

			void Draw(VkCommandBuffer buffer) const;

			void addToChildren(std::shared_ptr<GUIelement> child);
			bool isParent(std::shared_ptr<GUIelement> other) const;
			bool isChildOf(std::shared_ptr<GUIelement> other);

		protected:
			ke::prim::Rect mRectangle;

			std::vector<std::shared_ptr<GUIelement>> mChildren;
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