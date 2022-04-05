
#ifndef E_TEX_FONT_H
#define E_TEX_FONT_H

#include <nbug/core/str.h>
#include <nbug/core/file.h>
#include <nbug/core/ref.h>
#include <nbug/core/debug.h>

namespace e
{
	class Font;
	// abstract font
	class FontImp;
	class Font : public RefObject
	{
		friend class Graphics;
		friend class FontImp;
		//friend class Ref<Font>;
		FontImp * imp;
		Font(FontImp * _imp);
		~Font();
	public:
		static const int MAX_FONT_SIZE = 128 ;
		int W(Char _ch) const;
		int W(const Char * _buf, int _len) const;
		int H() const;
#ifdef E_CFG_LUA
		static bool Register(lua_State * L);
#endif
	};
	typedef Ref<Font> FontRef;
}

#endif


