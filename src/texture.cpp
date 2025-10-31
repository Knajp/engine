#include "texture.hpp"
#include "renderer.hpp"

ke::Texture::Texture(std::string filepath)
{
	ke::Renderer::getInstance().createTextureImage(mImage, mImageMemory, filepath);
	ke::Renderer::getInstance().createTextureImageView(mImageView, mImage);
}

VkImageView ke::Texture::getView() const
{
	return mImageView;
}


