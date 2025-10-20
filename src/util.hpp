#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <array>

namespace ke
{
	namespace util
	{
		std::vector<char> readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);;
			if (!file.is_open())
				std::cerr << "Utility error: failed to open file " << filename << "\n";

			size_t filesize = (size_t)file.tellg();
			std::vector<char> buffer(filesize);

			file.seekg(0);
			file.read(buffer.data(), filesize);

			return buffer;
		}
	}
	
	namespace str
	{
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;

			static VkVertexInputBindingDescription getInputBindingDescription()
			{
				VkVertexInputBindingDescription desc{};
				desc.binding = 0;
				desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				desc.stride = sizeof(Vertex);
			}

			static std::array<VkVertexInputAttributeDescription, 2> getInputAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 2> descs{};
				
				descs[0].binding = 0;
				descs[0].format = VK_FORMAT_R32G32_SFLOAT;
				descs[0].location = 0;
				descs[0].offset = offsetof(Vertex, pos);

				descs[1].binding = 0;
				descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				descs[1].location = 1;
				descs[1].offset = offsetof(Vertex, color);
			}
		};
	}
}