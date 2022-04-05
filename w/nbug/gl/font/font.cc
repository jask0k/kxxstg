
#include "../private.h"
#include <nbug/ui/win.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/font/font_.h>
#include <nbug/ui/win_.h>
#include <nbug/gl/font/font_loader.h>
#include <nbug/gl/graphics.h>
#include <nbug/gl/graphics_.h>
#include <nbug/core/env.h>

namespace e
{
	Font::Font(FontImp * _imp)
	{
		imp = _imp;
	}

	Font::~Font()
	{
		delete imp;
	}

	FontImp::FontImp(FontLoader * _loader, uint _lineHeight)
	{
		lineHeight = _lineHeight;
		loader = _loader;
		g = 0;
		//memset(chars, 0, sizeof(chars));
		currentPage = 0;
		glyphEnd = glyphMap.end();
	}

	FontImp::~FontImp()
	{
		E_SAFE_DELETE(loader);
		CleanAll();
		if(g)
		{
			Detach();
		}
	}

//	Font * Graphics::LoadFont(const Path & _path, uint _size)
//	{
//		//if(!_fs)
//		//{
//		//	E_ASSERT(0);
//		//	return 0;
//		//}
//
//		if(FS::IsFile(_path))
//		{
//#ifdef E_CFG_FREETYPE
//			//if(_fs != GetSystemFS())
//			//{
//			//	E_ASSERT(0);
//			//	return 0;
//			//}
//			FontLoader * p = FreeTypeFontLoader::Load(_path, _size);
//			if(p == 0)
//			{
//				E_ASSERT(0);
//				return 0;
//			}
//			FontImp * o = enew FontImp(p, _size);
//			return enew Font(o);
//#else
//			E_ASSERT(0);
//			return 0;
//#endif
//		}
//		else if(FS::IsFolder(_path))
//		{
//			if(!ImageFontLoader::PathIsImageFontFolder(_path))
//			{
//				E_ASSERT(0);
//				return 0;
//			}
//			ImageFontLoader * p = enew ImageFontLoader(_path);
//			FontImp * o = enew FontImp(p, p->lineHeight);
//			return enew Font(o);
//		}
//		else
//		{
//			E_ASSERT(0);
//			return 0;
//		}
//	}

	static Path _FindFontFile(const string & _fileName)
	{
		DirectoryRef it;
#ifdef NB_LINUX
		// look in font dirs
		it = FS::OpenDir(Env::GetHomeFolder() | ".fonts");
		if(it)
		{
			do
			{
				if(MatchFileName(it->GetName(), _fileName))
				{
					return it->GetFullPath();
				}
			}while(it->MoveNext(true));
		}

		it = FS::OpenDir(Path(L"/usr/share/fonts"));
		if(it)
		{
			do
			{
				if(MatchFileName(it->GetName(), _fileName))
				{
					return it->GetFullPath();
				}
			}while(it->MoveNext(true));
		}
#endif

#ifdef NB_WINDOWS
		// TODO: support system font folder
#endif
		it = FS::OpenDir(Env::GetDataFolder());
		if(it)
		{
			do
			{
				if(MatchFileName(it->GetName(), _fileName))
				{
					return it->GetFullPath();
				}
			}while(it->MoveNext(true));
		}
		return Path();
	}

	Font * Graphics::LoadFont(const string & _name, int _size, bool _bold)
	{
		FontLoader * loader = 0;
		bool hasDot = _name.find(L'.') != -1;
		bool hasSlash = _name.find_any(L"/\\") != -1;

		if(hasDot || hasSlash)
		{
			Path path;
			if(hasSlash)
			{
				path = _name;
			}
			else
			{
				path = _FindFontFile(_name);
			}

			if(path.IsValid())
			{
				if(FS::IsFile(path))
				{
#ifdef E_CFG_FREETYPE
					loader = FreeTypeFontLoader::Load(path, _size);
#else
					message(L"[nb] (WW) Graphics::LoadFont(): Direct load from file is unsupported.");
#endif
				}
				else if(ImageFontLoader::PathIsImageFontFolder(path))
				{
					loader = enew ImageFontLoader(path);
				}
				else
				{
					message(L"[nb] (WW) Graphics::LoadFont(): Invalid font path: \"" + path.GetBaseName() + L"\"");
				}
			}

			if(!loader)
			{
				message(L"[nb] Failed to load font: \"" + path.GetBaseName() + L"\"");
				loader = enew SystemFontLoader(_name, _size, _bold);
			}
		}
		else
		{
			loader = enew SystemFontLoader(_name, _size, _bold);
		}

		if(!loader)
		{
#	ifdef NB_WINDOWS
			const Char * defautFontName = L"Tahoma";
#	else
			const Char * defautFontName = L"Sans";
#	endif
			message(L"[nb] (WW) Graphics::LoadFont(): Load failed, fallback to \"" + string(defautFontName) + L"\"");
			loader = enew SystemFontLoader(defautFontName, 16, _bold);

		}
		E_ASSERT(loader != 0);
		return loader ? enew Font(enew FontImp(loader, _size)) : 0;
	}


	void FontImp::Attach(GraphicsImp * _g)
	{
		if(g)
		{
			Detach();
		}
		g = _g;
		next = g->fonts;
		g->fonts = this;
	}

	void FontImp::Detach()
	{
		CleanAll();

		FontImp * p0 = g->fonts;
		FontImp * p1 = 0;
		while(p0 != this && p0 != 0)
		{
			p1 = p0;
			p0 = p0->next;
		}
		if(p0 != 0)
		{
			if(p1)
			{
				p1->next = p0->next;
			}
			else
			{
				g->fonts = p0->next;
			}
		}
		else
		{
			E_ASSERT(0);
		}
		g = 0;
	}

	void FontImp::AllocNewPage()
	{
		E_ASSERT(g != 0);
		if(currentPage)
		{
			pages.push_back(currentPage);
			currentPage = 0;
		}
		currentX = 0;
		currentY = 0;
		currentPage = g->_CreateGlyphTex(FONT_TEX_W, FONT_TEX_H, this, pages.size());
	}

	TexGlyph * FontImp::LoadGlyph(Char _ch, bool _getSizeOnly)
	{
		// NB_PROFILE_INCLUDE;

		TexGlyph * ch = GetGlyph(_ch);
		if(ch == 0)
		{
			ch = enew TexGlyph;
			memset(ch, 0, sizeof(TexGlyph));
			glyphMap[_ch] = ch;
			glyphEnd = glyphMap.end();
		}

		E_ASSERT(!ch->Loaded());

		Image pic;
		if(!loader->LoadGlyph(_ch, pic))
		{
			// white space
			pic.Alloc(lineHeight / 2, lineHeight);
			memset(pic.data, 0x00, pic.w * pic.h * 4);
			for(uint i=1; i<pic.w-1; i++)
			{
				uint8 * p;
				p = pic.Get(i, 1);
				p[0] = p[1] = p[2] = p[3] = 0xff;
				p = pic.Get(i, pic.h - 2);
				p[0] = p[1] = p[2] = p[3] = 0xff;
			}
			for(uint i=1; i<pic.h-1; i++)
			{
				uint8 * p;
				p = pic.Get(1, i);
				p[0] = p[1] = p[2] = p[3] = 0xff;
				p = pic.Get(pic.w - 2, i);
				p[0] = p[1] = p[2] = p[3] = 0xff;
			}
		}
		E_ASSERT(pic.w > 0 && pic.w <= Font::MAX_FONT_SIZE);
		E_ASSERT(pic.h > 0 && pic.h <= Font::MAX_FONT_SIZE);
		if(lineHeight < (int)pic.h)
		{
			lineHeight = pic.h;
		}
		ch->advance = pic.w;

		if(_getSizeOnly)
		{
			return ch;
		}

		E_ASSERT(g != 0);
		if(!currentPage)
		{
			AllocNewPage();
		}
		if(currentX + pic.w >= FONT_TEX_W)
		{
			// current line full, switch to next line
			currentX = 0;
			currentY+= lineHeight + 1;
			//Image pic;
			//currentPage->GetImage(pic);
			//pic.Save(Env::GetDataFolder() | L"fontdump.png", Image::PNG);
		}

		if(currentY + lineHeight >= FONT_TEX_H)
		{
			// page full, alocate anthor
			AllocNewPage();
		}

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			GLuint t = currentPage->GetNative();
			glBindTexture(GL_TEXTURE_2D, t);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
			glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, pic.w, pic.h, GL_RGBA, GL_UNSIGNED_BYTE, pic.data);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			RECT dst;
			dst.left = currentX;
			dst.top = currentY;
			dst.right = dst.left + pic.w - 1;
			dst.bottom = dst.top + pic.h - 1;
			RECT src;
			src.left = 0;
			src.top = 0;
			src.right = pic.w - 1;
			src.bottom = pic.h - 1;
			D3DLOCKED_RECT lockedRect;
			IDirect3DTexture9 * t = DXTEX(currentPage->GetNative());
			if(SUCCEEDED(t->LockRect(0, &lockedRect, NULL, 0)))
			{
				uint8 * p = (uint8 *)lockedRect.pBits;
				p = p + currentY * lockedRect.Pitch + currentX * 4;
				for(uint y=0; y< pic.h; y++)
				{
					memcpy(p, pic.Get(0, y), pic.w * 4);
					p+= lockedRect.Pitch;
				}
				t->UnlockRect(0);
			}
			else
			{
				E_ASSERT(0);
			}
#endif
		}

		ch->tex = currentPage;
		ch->x0 = (float)(currentX) / FONT_TEX_W;
		ch->y0 = (float)(currentY) / FONT_TEX_H;
		ch->x1 = (float)(currentX+pic.w) / FONT_TEX_W;
		ch->y1 = (float)(currentY+pic.h) / FONT_TEX_H;
		currentX+= pic.w + 1;
		return ch;
	}

	void FontImp::CleanAll()
	{
		// memset(chars, 0, sizeof(chars));
		for(GlyphMap::iterator it = glyphMap.begin(); it != glyphMap.end(); ++it)
		{
			delete it->second;
		}
		glyphMap.clear();
		glyphEnd = glyphMap.end();
		for(uint i=0; i<pages.size(); i++)
		{
			pages[i].Detach();
		}
		pages.clear();
		currentPage.Detach();
	}

	inline int Font::W(Char _ch) const
	{
		TexGlyph * glyph = imp->GetGlyph(_ch);
		if(glyph == 0)
		{
			glyph = imp->LoadGlyph(_ch, true);
		}
		return glyph->advance;
	}

	int Font::W(const Char * _buf, int _len) const
	{
		int ret = 0;
		for(int i = 0; i < _len; i++)
		{
			ret+= W(_buf[i]);
		}
		return ret;
	}

	int Font::H() const
	{
		return imp->lineHeight;
	}

#ifdef E_CFG_LUA
#define CLASSNAME "Font"

	static Font * LuaCheckFont(lua_State * L, int _index)
	{
		int type = lua_type(L, _index);
		if(type == LUA_TUSERDATA)
		{
			void *ud = luaL_checkudata(L, _index, CLASSNAME);
			if(ud == 0)
			{
				luaL_typerror(L, _index, CLASSNAME);
			}
			return *(Font**)ud;
		}
		else if(type != LUA_TNIL)
		{
			luaL_typerror(L, _index, CLASSNAME);
		}

		return 0;
	}

	static int Font_gc(lua_State * L)
	{
		Font * f = LuaCheckFont(L, 1);
		if(f)
		{
			f->Release();
		}
		return 0;
	}

	//static int Font_W(lua_State * L)
	//{
	//	Font * tex = LuaCheckFont(L, 1);
	//	int w = tex ? tex->W() : 0;
	//	lua_pushinteger(L, w);
	//	return 1;
	//}

	//static int Font_H(lua_State * L)
	//{
	//	Font * tex = LuaCheckFont(L, 1);
	//	int h = tex ? tex->H() : 0;
	//	lua_pushinteger(L, h);
	//	return 1;
	//}

	static int Font_Extents(lua_State * L)
	{
		Font * f = LuaCheckFont(L, 1);
		if(f)
		{
			const char * s = lua_tostring(L, 2);
			string s1(s);
			int w = f ? f->W(s1.c_str(), s1.length()) : 0;
			lua_pushinteger(L, w);
			int h = f ? f->H() : 0;
			lua_pushinteger(L, h);
		}
		else
		{
			lua_pushinteger(L, 1);
			lua_pushinteger(L, 1);
		}
		return 2;
	}

	static const luaL_reg Font_methods[] =
	{
		{"Extents", &Font_Extents},
//		{"H", &Font_H},
		{0, 0}
	};

	int LuaSetFont(lua_State * L)
	{
		Font * f = *(Font**) lua_touserdata(L, 1);
		Graphics::Singleton()->SetFont(f);
		return 0;
	}

	int LuaLoadFont(lua_State * L)
	{
		//Font * LoadFont(const Path & _path, uint _size = 16);
		//Font * LoadFont(const string & _name, int _size, bool _bold = false);

		int n = lua_gettop(L);
		E_ASSERT(n  >= 2 && n <= 3);
		const char * name = lua_tostring(L, 1);
		int size = lua_tointeger(L, 2);
		bool bold = false;

		if(n >= 3)
		{
			bold = lua_toboolean(L, 3) ? true : false;
		}

		Font * font = Graphics::Singleton()->LoadFont(name, size, bold);
		if(font)
		{
			font->AddRef();
			void * ud = lua_newuserdata(L, sizeof(Font *));
			*(Font**)ud = font;
			luaL_getmetatable(L, CLASSNAME);
			lua_setmetatable(L, -2);
		}
		else
		{
			lua_pushnil(L);
		}
		return 1;
	}

	bool Font::Register(lua_State * L)
	{
		lua_newtable(L);
		int methodtable = lua_gettop(L);
		luaL_newmetatable(L, CLASSNAME);
		int metatable = lua_gettop(L);

		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, methodtable);
		lua_settable(L, metatable);

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methodtable);
		lua_settable(L, metatable);

		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, Font_gc);
		lua_settable(L, metatable);

		lua_pop(L, 1);

		luaL_openlib(L, 0, Font_methods, 0);
		lua_pop(L, 1);

		lua_register(L, "Font", &LuaLoadFont);
		lua_register(L, "SetFont", &LuaSetFont);

		return true;
	}
#endif

}
