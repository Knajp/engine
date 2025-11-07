#include "text.hpp"

ke::text::Text& ke::text::Text::getInstance()
{
	static ke::text::Text instance;
	return instance;
}

ke::text::Text::Text()
{
	if (FT_Init_FreeType(&mLibrary))
		mLogger.error("Failed to initialize FreeType!");
}

ke::text::Font::Font(std::string path, FT_Library& lib)
{
	if (FT_New_Face(lib, path.c_str(), 0, &mFace))
}
