
#pragma once

#include <nbug/core/str.h>
#include <nbug/core/file.h>

namespace e
{
	class Image;
	class FontLoader
	{
	public:
		virtual ~FontLoader() {};
		virtual bool LoadGlyph(Char _ch, Image & _pic) = 0;
	};

	// syatem font. HFONT on windows, xft on Linux(may be slow?)
	class SystemFontLoaderImp;
	class SystemFontLoader : public FontLoader
	{
		SystemFontLoaderImp * imp2;
	public:
		bool LoadGlyph(Char _ch, Image & _pic) override;
		SystemFontLoader(const string & _fontName, int _fontSize, bool _bold = false);
		~SystemFontLoader();
	};

	// picture (pixel) font
	class ImageFontLoaderImp;
	class ImageFontLoader : public FontLoader
	{
		ImageFontLoaderImp * imp2;
	public:
		int spaceWidth;
		int lineHeight;
		bool LoadGlyph(Char _ch, Image & _pic) override;
		ImageFontLoader(const Path & _folder);
		~ImageFontLoader();
		static bool PathIsImageFontFolder(const Path & _path);
	};

#ifdef E_CFG_FREETYPE
	// fonts load by FreeType library, *.ttf
	class FreeTypeFontLoaderImp;
	class FreeTypeFontLoader : public FontLoader
	{
		FreeTypeFontLoaderImp * imp2;
		FreeTypeFontLoader(FreeTypeFontLoaderImp * _p);
	public:
		~FreeTypeFontLoader();
		bool LoadGlyph(Char _ch, Image & _pic) override;
		static FreeTypeFontLoader * Load(const Path & _path, uint _size);
	};
#endif

}
