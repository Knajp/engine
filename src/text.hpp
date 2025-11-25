#pragma once
#define FT_FREETYPE_H
#include <ft2build.h>
#include <freetype/freetype.h>

#include "logger.hpp"
#include "util.hpp"
#include <unordered_map>

namespace ke
{
	namespace text
	{	
		inline ke::Logger textLogger = ke::Logger("Text logger", spdlog::level::info);

		constexpr std::string_view FONT_PATH_GOTHIC = "fonts/GOTHIC.TTF";
		class Font
		{
		public:
			Font(std::string path, FT_Library& lib);

			std::map<char, ke::str::Glyph> glyphs;

		private:
			FT_Face mFace;

			VkImage mImage;
			VkImageView mImageView;
			VkDeviceMemory mImageMemory;
		};
		class Text
		{
		public:
			static Text& getInstance();

			void buildTextVertices(const std::string& text, const std::string_view font, glm::vec2 position, glm::vec3 color);
			void loadFont(std::string path, std::string key);
		private:
			Text();

			FT_Library mLibrary;
			VkSampler mTextSampler;
			VkDescriptorSetLayout mSetLayout;
			std::vector<VkDescriptorSet> mSets;


			std::unordered_map<std::string_view, std::shared_ptr<Font>> mFontMap;
		};
		class TextLabel
		{
		public:
			TextLabel();

		private:

			std::vector<ke::str::Vertex> mVertices;
			std::vector<uint16_t> mIndices;

		};
		
	}
}