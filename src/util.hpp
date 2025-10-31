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
		inline std::vector<char> readFile(const std::string& filename)
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
			glm::vec2 uv;

			static VkVertexInputBindingDescription getInputBindingDescription()
			{
				VkVertexInputBindingDescription desc{};
				desc.binding = 0;
				desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				desc.stride = sizeof(Vertex);

				return desc;
			}

			static std::array<VkVertexInputAttributeDescription, 3> getInputAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 3> descs{};
				
				descs[0].binding = 0;
				descs[0].format = VK_FORMAT_R32G32_SFLOAT;
				descs[0].location = 0;
				descs[0].offset = offsetof(Vertex, pos);

				descs[1].binding = 0;
				descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				descs[1].location = 1;
				descs[1].offset = offsetof(Vertex, color);

				descs[2].binding = 0;
				descs[2].format = VK_FORMAT_R32G32_SFLOAT;
				descs[2].location = 2;
				descs[2].offset = offsetof(Vertex, uv);

				return descs;
			}
		};

		struct MouseProperties
		{
			bool isPressed;
			glm::vec2 cursorPosition;
		};

		struct MVP
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};
		
		struct PushConstants
		{
			int useTexture;
		};
	}
}