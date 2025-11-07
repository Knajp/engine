#pragma once
#include <ft2build.h>
#include <freetype/freetype.h>
#define FT_FREETYPE_H
#include "logger.hpp"
namespace ke
{
	namespace text
	{	
		class Text
		{
		public:
			static Text& getInstance();

		private:
			Text();

			FT_Library mLibrary;
			ke::Logger mLogger = ke::Logger("Text logger", spdlog::level::info);
		};
		class Font
		{
		public:
			Font(std::string path, FT_Library& lib);

		private:
			FT_Face mFace;
		};
	}
}