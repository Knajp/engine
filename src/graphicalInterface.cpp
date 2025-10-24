#include "graphicalInterface.hpp"

ke::ui::GUIelement::GUIelement(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color)
	:	mRectangle(_position, _extent, _color)
{

}

void ke::ui::GUIelement::Draw(VkCommandBuffer buffer) const
{
	mRectangle.Draw(buffer);
}

ke::ui::Button::Button(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color)
	: GUIelement(_position, _extent, _color)
{
}

bool ke::ui::Button::isHovering(glm::vec2 cursorPositon) const
{
	return mRectangle.intersects(cursorPositon);
}

