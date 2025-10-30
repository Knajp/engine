#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

namespace ke
{
	class Texture
	{
		Texture() = default;
		Texture(std::string filepath);

	private:
		VkImage mImage;
		VkDeviceMemory mImageMemory;
		VkImageView mImageView;

	};
}