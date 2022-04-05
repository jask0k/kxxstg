

#ifdef E_CFG_FREETYPE

#include "../private.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <nbug/gl/font/font_loader.h>
#include <nbug/core/debug.h>
#include <nbug/core/file.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/image.h>
#include <nbug/core/env.h>

namespace e
{
	class FreeTypeFontLoaderImp
	{
	public:
		uint size;
		FT_Library library;
		FT_Face face;
	};

	FreeTypeFontLoader::FreeTypeFontLoader(FreeTypeFontLoaderImp * _p)
	{
		imp2 = _p;
	}

	FreeTypeFontLoader::~FreeTypeFontLoader()
	{
		if(imp2)
		{
			FT_Done_Face(imp2->face);
			FT_Done_FreeType(imp2->library);
			delete imp2;
		}
	}

	FreeTypeFontLoader * FreeTypeFontLoader::Load(const Path & _path, uint _size)
	{
		FreeTypeFontLoaderImp * imp2 = enew FreeTypeFontLoaderImp;
		FT_Error error = FT_Init_FreeType(&imp2->library);
		if(error)
		{
			delete imp2;
			imp2 = 0;
			return 0;
		}
		error = FT_New_Face(imp2->library, _path.GetStringA().c_str(), 0, &imp2->face);
		if(error)
		{
			E_ASSERT1(0, "[nb] load free type font failed:" + _path.GetStringA());
			FT_Done_FreeType(imp2->library);
			delete imp2;
			imp2 = 0;
			return 0;
		}

		imp2->size = _size;
		E_TRACE_LINE("[nb] free type font is loaded: " + _path.GetStringA());
		E_TRACE_LINE("\tnum_glyphs = " + string((int)imp2->face->num_glyphs));
		E_TRACE_LINE("\tface_flags = " + string((int)imp2->face->face_flags));
		E_TRACE_LINE("\tunits_per_EM = " + string((int)imp2->face->units_per_EM));
		E_TRACE_LINE("\tnum_fixed_sizes = " + string((int)imp2->face->num_fixed_sizes));
		
		return enew FreeTypeFontLoader(imp2);;
	}

	bool FreeTypeFontLoader::LoadGlyph(Char _ch, Image & _pic)
	{
		if(imp2 == 0)
		{
			return false;
		}

		FT_Error error;
		error = FT_Set_Pixel_Sizes(imp2->face, 0, imp2->size);
		if(error)
		{
			return false;
		}

		int glyph_index = FT_Get_Char_Index(imp2->face, _ch);
		if(glyph_index == 0)
		{
			return false;
		}

		error = FT_Load_Glyph(imp2->face,  glyph_index, FT_LOAD_DEFAULT);
		if(error)
		{
			return false;
		}

		FT_GlyphSlot slot = imp2->face->glyph;
		error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
		if(error)
		{
			return false;
		}

		int byteWidth = (slot->bitmap.width + 7) / 8;
		int bearingX = slot->metrics.horiBearingX >> 6;
		int bearingY = slot->metrics.horiBearingY >> 6;
		int x0 = slot->bitmap_left;
		int y0 = -(slot->bitmap_top - slot->bitmap.rows);
		int advance = (slot->advance.x >> 6);
		int w = advance;
		int h = imp2->face->size->metrics.height >> 6;
		int a = imp2->face->size->metrics.ascender >> 6;
		int b = imp2->face->size->metrics.descender >> 6;
		//_h = slot->bitmap.rows + 2;
		if(w == 0 || w > Font::MAX_FONT_SIZE 
			|| h == 0 || h > Font::MAX_FONT_SIZE)
		{
			return false;
		}

		_pic.Alloc(w, h);
		_pic.Fill(0x00ffffff);

		switch(slot->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			for(int y=0; y<slot->bitmap.rows; y++)
			{
				for(int x=0; x<slot->bitmap.width; x++)
				{
					//uint8 & d = _buf[_w * (_h + b - bearingY + y) + bearingX + x];
					int x1 = bearingX + x;
					int y1 = h + b - bearingY + y;
					if(x1 > 0 && x1 < (int)_pic.w && y1 > 0 && y1 < (int)_pic.h)
					{
						uint8 s = slot->bitmap.buffer[y * slot->bitmap.pitch + x / 8];
						int i = 7 - x % 8;
						uint8 alpha = ((s >> i) & 0x01) ? 0xff : 0;
						if(alpha)
						{
							uint8 * d = _pic.Get(bearingX + x, h + b - bearingY + y);
							d[0] = 255;
							d[1] = 255;
							d[2] = 255;
							d[3] = alpha;
						}
					}
				}
			}
			break;
		case FT_PIXEL_MODE_GRAY:
			for(int y=0; y<slot->bitmap.rows; y++)
			{
				for(int x=0; x<slot->bitmap.width; x++)
				{
					int x1 = bearingX + x;
					int y1 = h + b - bearingY + y;
					if(x1 > 0 && x1 < (int)_pic.w && y1 > 0 && y1 < (int)_pic.h)
					{
						uint8 s = slot->bitmap.buffer[y * slot->bitmap.pitch + x];
						if(s)
						{
							uint8 * d = _pic.Get(x1, y1);
							d[0] = 255; 
							d[1] = 255; 
							d[2] = 255; 
							d[3] = s;
						}
					}
				}
			}
			break;
		default:
			return false;
		}
		//_pic.Save(Env::GetDataFolder() | L"freetype.png", Image::PNG);
		return true;
	}
}
#else
	int g_nb_freetype_unsupported_dummy;
#endif // E_CFG_FREETYPE
