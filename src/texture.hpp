#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

namespace ke
{
	class Texture
	{
	public:
		Texture() = default;
		Texture(std::string filepath);

		VkImageView getView() const;
	private:
		VkImage mImage;
		VkDeviceMemory mImageMemory;
		VkImageView mImageView;

	};
}