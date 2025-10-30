#include "Rect.hpp"
#include "renderer.hpp"

ke::prim::Rect::Rect(glm::vec2 _xy, glm::vec2 _extent, glm::vec3 _color)
	:xy(_xy), extent(_extent), color(_color)
{
	std::vector<ke::str::Vertex> vertices =
	{
		{{_xy.x, _xy.y},						 {_color.r, _color.g, _color.b},	{0.0f, 0.0f}},
		{{_xy.x + _extent.x, _xy.y},			 {_color.r, _color.g, _color.b},	{1.0f, 0.0f}},
		{{_xy.x, _xy.y + _extent.y},			 {_color.r, _color.g, _color.b},	{0.0f, 1.0f}},
		{{_xy.x + _extent.x, _xy.y + _extent.y}, {_color.r, _color.g, _color.b},	{1.0f, 1.0f}}
	};

	std::vector<uint16_t> indices =
	{
		0, 1, 2,
		2, 3, 1
	};

	mVertices = vertices;
	mIndices = indices;

	createVulkanBuffers();
}

ke::prim::Rect::~Rect()
{
	ke::Renderer::getInstance().submitBufferForDestruction(std::make_pair(mVertexBuffer, mVertexBufferMemory));
	ke::Renderer::getInstance().submitBufferForDestruction(std::make_pair(mIndexBuffer, mIndexBufferMemory));
}

ke::prim::Rect::Rect(const Rect& other)
{
	xy = other.xy;
	extent = other.extent;
	color = other.color;	
	mVertices = other.mVertices;
	mIndices = other.mIndices;

	createVulkanBuffers();
}

void ke::prim::Rect::operator=(const Rect& other)
{
	if (this == &other) return;

	xy = other.xy;
	extent = other.extent;
	color = other.color;
	mVertices = other.mVertices;
	mIndices = other.mIndices;

	if(mVertexBuffer != VK_NULL_HANDLE)
		ke::Renderer::getInstance().submitBufferForDestruction(std::make_pair(mVertexBuffer, mVertexBufferMemory));
	if(mIndexBuffer != VK_NULL_HANDLE)
		ke::Renderer::getInstance().submitBufferForDestruction(std::make_pair(mIndexBuffer, mIndexBufferMemory));

	createVulkanBuffers();
}

void ke::prim::Rect::Draw(VkCommandBuffer commandBuffer) const
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mIndices.size()), 1, 0, 0, 0);
}

bool ke::prim::Rect::intersects(glm::vec2 point) const
{
	return (point.x >= xy.x && point.x <= xy.x + extent.x) && (point.y >= xy.y && point.y <= xy.y + extent.y);
}

void ke::prim::Rect::createVulkanBuffers()
{
	ke::Renderer::createVertexBuffer(mVertexBuffer, mVertexBufferMemory, mVertices);
	ke::Renderer::createIndexBuffer(mIndexBuffer, mIndexBufferMemory, mIndices);
}


