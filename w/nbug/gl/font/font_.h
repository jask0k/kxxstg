
#pragma once

#include <nbug/tl/map.h>
#include <nbug/core/debug_.h>

namespace e
{
	struct TexGlyph
	{
		Tex * tex;
		float x0, y0, x1, y1;
		int advance;

		bool Loaded() const { return tex != 0; }
	};

	class FontLoader;
	class GraphicsImp;
	class FontImp
	{
	public:
		static const int FONT_TEX_W = 128;
		static const int FONT_TEX_H = 128;

		FontLoader * loader;
		GraphicsImp * g;
		FontImp * next;
		int lineHeight;

		typedef e::Map<Char, TexGlyph*> GlyphMap;
		GlyphMap glyphMap;
		GlyphMap::iterator glyphEnd;
		// cache (texture) page
		Array<TexRef> pages; // currentTexture is excluded
		TexRef  currentPage;
		int currentX;
		int currentY;

		FontImp(FontLoader * _loader, uint _lineHeight);
		~FontImp();
		void AllocNewPage();
		void CleanAll();
		TexGlyph * LoadGlyph(Char _ch, bool _getSizeOnly);
		void Attach(GraphicsImp * _g);
		void Detach();
		TexGlyph * GetGlyph(Char _ch)
		{
			GlyphMap::iterator it = glyphMap.find(_ch);
			E_BASIC_ASSERT(glyphEnd == glyphMap.end());
			return (it == glyphEnd) ? 0 : it->second;
		}
	};
}

