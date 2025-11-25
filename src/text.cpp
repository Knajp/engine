#include "text.hpp"
#include "renderer.hpp"

ke::text::Text& ke::text::Text::getInstance()
{
	static ke::text::Text instance;
	return instance;
}

void ke::text::Text::buildTextVertices(const std::string& text, const std::string_view _font, glm::vec2 position, glm::vec3 color)
{
	std::vector<ke::str::Vertex> vertices;
	float x = position.x;
	float y = position.y;

	std::shared_ptr<Font> font = mFontMap[_font];
	for (char c : text)
	{
		const ke::str::Glyph& g = font->glyphs[c];

		float xMin = x + g.bearingX;
		float yMax = y - g.bearingY;

		float w = g.uvW;
		float h = g.uvH;

		float xMax = xMin + w;
		float yMin = yMax + h;

		vertices.push_back(ke::str::Vertex({ xMin, yMax }, color, { g.uvX,         g.uvY }));
		vertices.push_back(ke::str::Vertex({ xMax, yMax }, color, { g.uvX + g.uvW, g.uvY }));
		vertices.push_back(ke::str::Vertex({ xMax, yMin }, color, { g.uvX + g.uvW, g.uvY + g.uvH }));

		vertices.push_back(ke::str::Vertex({ xMin, yMax }, color, { g.uvX,         g.uvY }));
		vertices.push_back(ke::str::Vertex({ xMax, yMin }, color, { g.uvX + g.uvW, g.uvY + g.uvH }));
		vertices.push_back(ke::str::Vertex({ xMin, yMin }, color, { g.uvX,         g.uvY + g.uvH }));

		x += g.advance;
	}
}

void ke::text::Text::loadFont(std::string path, std::string key)
{
	mFontMap[key] = std::make_shared<Font>(path, mLibrary);
}

ke::text::Text::Text()
{
	if (FT_Init_FreeType(&mLibrary))
		ke::text::textLogger.error("Failed to initialize FreeType!");

	ke::Renderer::getInstance().createTextSampler(mTextSampler);
	ke::Renderer::getInstance().createTextSetLayout(mSetLayout);
}

ke::text::Font::Font(std::string path, FT_Library& lib)
{
	if (FT_New_Face(lib, path.c_str(), 0, &mFace))
		ke::text::textLogger.error("Failed to create font face!");

	const int ATLASWIDTH = 1024, ATLASHEIGHT = 1024;
	unsigned char* atlasData = new unsigned char[ATLASWIDTH * ATLASHEIGHT];
	memset(atlasData, 0, ATLASWIDTH * ATLASHEIGHT);

	int x = 0, y = 0, rowHeight = 0;

	for (unsigned char c = 32; c < 128; c++)
	{
		if (FT_Load_Char(mFace, c, FT_LOAD_RENDER))
			continue;

		FT_GlyphSlot g = mFace->glyph;
		if (x + g->bitmap.width >= ATLASWIDTH)
		{
			x = 0;
			y += rowHeight;
			rowHeight = 0;
		}

		for (unsigned int row = 0; row < g->bitmap.rows; row++)
			memcpy(atlasData + (y + row) * ATLASWIDTH + x, g->bitmap.buffer + row * g->bitmap.width, g->bitmap.width);

		glyphs[c] = {
			.uvX = (float)x / ATLASWIDTH,
			.uvY = (float)y / ATLASHEIGHT,
			.uvW = (float)g->bitmap.width / ATLASWIDTH,
			.uvH = (float)g->bitmap.rows / ATLASHEIGHT,
			.bearingX = g->bitmap_left,
			.bearingY = g->bitmap_top,
			.advance = g->advance.x >> 6
		};

		x += g->bitmap.width + 1;
		rowHeight = std::max(rowHeight, (int)g->bitmap.rows);

	}

	ke::Renderer::getInstance().createFontAtlasImage(mImage, mImageMemory, atlasData);
	ke::Renderer::getInstance().createFontAtlasView(mImageView, mImage);

}
