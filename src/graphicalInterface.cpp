#include "graphicalInterface.hpp"
#include "renderer.hpp"

ke::ui::GUIelement::GUIelement(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color)
	:	mRectangle(_position, _extent, _color)
{

}

void ke::ui::GUIelement::Draw(VkCommandBuffer buffer) const
{
	mRectangle.Draw(buffer);

	for (const auto& child : mChildren)
		child.get()->Draw(buffer);
}

void ke::ui::GUIelement::addToChildren(std::shared_ptr<GUIelement> child)
{
	if (!isChildOf(child))
		mChildren.push_back(child);
	else ke::ui::logger.warn("Attempted to add parent to children!");
}

bool ke::ui::GUIelement::isParent(std::shared_ptr<GUIelement> other) const
{
	return std::find(mChildren.begin(), mChildren.end(), other) != mChildren.end();
}

bool ke::ui::GUIelement::isChildOf(std::shared_ptr<GUIelement> other)
{
	if (!other) return false;

	return other->isParent(shared_from_this());
}

ke::ui::Button::Button(glm::vec2 _position, glm::vec2 _extent, glm::vec3 _color)
	: GUIelement(_position, _extent, _color)
{
}

bool ke::ui::Button::isHovering(glm::vec2 cursorPositon) const
{
	return mRectangle.intersects(cursorPositon);
}

