#include "texture.hpp"
#include "renderer.hpp"

ke::Texture::Texture(std::string filepath)
{
	ke::Renderer::getInstance().createTextureImage(mImage, mImageMemory);
	ke::Renderer::getInstance().createTextureImageView(mImageView, mImage);
}


