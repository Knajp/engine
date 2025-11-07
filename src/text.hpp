#pragma once
#include <ft2build.h>
#include <freetype/freetype.h>
#define FT_FREETYPE_H
#include "logger.hpp"
#include "util.hpp"
namespace ke
{
	namespace text
	{	
		ke::Logger textLogger = ke::Logger("Text logger", spdlog::level::info);
		class Text
		{
		public:
			static Text& getInstance();

		private:
			Text();

			FT_Library mLibrary;
		};
		class Font
		{
		public:
			Font(std::string path, FT_Library& lib);

		private:
			FT_Face mFace;

			VkImage mImage;
			VkImageView mImageView;
			VkDeviceMemory mImageMemory;
		};
	}
}