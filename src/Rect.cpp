#include "Rect.hpp"
#include "renderer.hpp"

ke::prim::Rect::Rect(glm::vec2 _xy, glm::vec2 _extent)
	:xy(_xy), extent(_extent)
{
	std::vector<ke::str::Vertex> vertices =
	{
		{{_xy.x, _xy.y}, {1.0f, 0.0f, 0.0f}},
		{{_xy.x + _extent.x, _xy.y}, {1.0f, 1.0f, 0.0f}},
		{{_xy.x, _xy.y + _extent.y}, {0.0f, 1.0f, 0.0f}},
		{{_xy.x + _extent.x, _xy.y + _extent.y}, {0.0f, 0.0f, 1.0f}}
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

void ke::prim::Rect::Draw(VkCommandBuffer commandBuffer) const
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mIndices.size()), 1, 0, 0, 0);
}

void ke::prim::Rect::createVulkanBuffers()
{
	ke::Renderer::createVertexBuffer(mVertexBuffer, mVertexBufferMemory, mVertices);
	ke::Renderer::createIndexBuffer(mIndexBuffer, mIndexBufferMemory, mIndices);
}


