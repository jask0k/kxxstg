
#include "../private.h"

#ifdef NB_LINUX
#   include <X11/Xft/Xft.h>
#   include <X11/Xlib.h>
#endif
#include <nbug/gl/font/font_loader.h>
#include <nbug/core/debug.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/image.h>
#include <nbug/core/env.h>

namespace e
{
	class SystemFontLoaderImp
	{
	public:
		SystemFontLoaderImp(const string & _fontName, int _fontSize, bool _bold);
		~SystemFontLoaderImp();
		bool LoadGlyph(Char _ch, Image & _pic);
		static int _refCount;
#ifdef NB_WINDOWS
		HFONT nativeFont;
		static HDC _hDC;
		static HBITMAP _hBitmap;
#endif

#ifdef NB_LINUX
		XftFont * nativeFont;
		static GC gc;
		static Drawable drawable;
		static XftDraw * xft;
		static XColor blackColor;
		static XColor whiteColor;
		static XftColor xftColor;
		static XImage * image;
		static int depth;
		typedef uint8 Palette[2]; // gray, allocated
		static Palette * local_palette;
		static uint32 local_palette_size;
#endif
	};

	int SystemFontLoaderImp::_refCount = 0;
#ifdef NB_WINDOWS
	HDC SystemFontLoaderImp::_hDC = 0;
	HBITMAP SystemFontLoaderImp::_hBitmap = 0;
#endif

#ifdef NB_LINUX

	extern Display * _display;
	extern int _screenNum;
	extern Colormap _screenColormap;

	GC SystemFontLoaderImp::gc = 0;
	Drawable SystemFontLoaderImp::drawable = 0;
	XftDraw * SystemFontLoaderImp::xft = 0;
	XColor SystemFontLoaderImp::blackColor;
	XColor SystemFontLoaderImp::whiteColor;
	XftColor SystemFontLoaderImp::xftColor;
	XImage * SystemFontLoaderImp::image = 0;
	int SystemFontLoaderImp::depth;
	SystemFontLoaderImp::Palette * SystemFontLoaderImp::local_palette = 0;
	uint32 SystemFontLoaderImp::local_palette_size = 0;
#endif

#ifdef NB_WINDOWS
	static HFONT CreateNativeFont(const string & _fontName, int _fontSize, bool _bold)
	{
		if(_fontSize < 1)
		{
			_fontSize = 1;
		}
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));

		lf.lfHeight = -_fontSize;
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		if(_bold)
		{
			lf.lfWeight = FW_BOLD;
		}
		else
		{
			lf.lfWeight = FW_NORMAL;
		}

		//lf.lfItalic    = _font.italic ? TRUE : FALSE;
		//lf.lfUnderline = _font.underline ? TRUE : FALSE;
		//lf.lfStrikeOut = _font.strikethrough ? TRUE : FALSE;
		lf.lfCharSet   = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = ANTIALIASED_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH;
		if(_fontName.length() > 32)
		{
#	ifdef UNICODE
			wcscpy(lf.lfFaceName, L"SYSTEM");
#	else
			strcpy(lf.lfFaceName, "SYSTEM");
#	endif
		}
		else
		{
#	ifdef UNICODE
			wcscpy(lf.lfFaceName, _fontName.c_str());
#	else
			strcpy(lf.lfFaceName, _fontName.c_str());
#	endif
		}
		return ::CreateFontIndirect(&lf);
	}
#endif

#ifdef NB_LINUX
	static XftFont * CreateNativeFont(const string & _fontName, int _fontSize, bool _bold)
	{
		// ** NOTE **
		// valgrind may report memory leaks for XftFontOpen(), but according to xft man page:
		// "XftFonts are internally allocated, reference-counted, and freed by Xft,
		// the programmer does not ordinarily need to allocate or free storage for them."

		stringa fontName(_fontName);
		XftFont * f = 0;
		//if(_bold && _font.italic)
		//{
		//	f = ::XftFontOpen(_display, _screenNum,
		//			  XFT_FAMILY,  XftTypeString, (const char *)fontName.c_str(),
		//			  XFT_SIZE,    XftTypeInteger, (int)_fontSize,
		//			  XFT_STYLE,   XftTypeString, "Bold",
		//			  XFT_WEIGHT,  XftTypeDouble, 200.0,
		//			  NULL);
		//}
		//else
		if(_bold)
		{
			f = ::XftFontOpen(_display, _screenNum,
					  XFT_FAMILY,  XftTypeString, (const char *)fontName.c_str(),
					  XFT_SIZE,    XftTypeInteger, (int)_fontSize,
					  XFT_STYLE,   XftTypeString, "Bold",
					  XFT_WEIGHT,  XftTypeDouble, 200.0,
					  XFT_SLANT,   XftTypeDouble, 100.0,
					  NULL);
		}
		//else if(_font.italic)
		//{
		//	f = ::XftFontOpen(_display, _screenNum,
		//			  XFT_FAMILY,  XftTypeString, (const char *)fontName.c_str(),
		//			  XFT_SIZE,    XftTypeInteger, (int)_fontSize,
		//			  XFT_SLANT,   XftTypeDouble, 100.0,
		//			  NULL);
		//}
		else
		{
			f = ::XftFontOpen(_display, _screenNum,
					  XFT_FAMILY,  XftTypeString, (const char *)fontName.c_str(),
					  XFT_SIZE,    XftTypeInteger, (int)_fontSize,
					  NULL); // 3,936 (1,536 direct, 2,400 indirect) bytes in 2 blocks are definitely lost in loss record 307 of 354 [PID: 26939]	sys_font_loader.cc	/w/nbug/gl/font	line 163	Valgrind Problem

		}
		return f;
	}
#endif
	SystemFontLoader::SystemFontLoader(const string & _fontName, int _fontSize, bool _bold)
	{
		imp2 = enew SystemFontLoaderImp(_fontName, _fontSize, _bold);
	}

	SystemFontLoaderImp::SystemFontLoaderImp(const string & _fontName, int _fontSize, bool _bold)
	{
		if(_refCount == 0)
		{
			//E_TRACE_LINE("[nb] Create first system font face, alloc drawing buffer.");
#ifdef NB_WINDOWS
		//	HWND hWnd = ::GetForegroundWindow();
			//HWND hWnd = 0;
			//if(hWnd == NULL)
			//{
			//	hWnd = ::GetDesktopWindow();
			//	E_ASSERT(hWnd != NULL);
			//}
			//HDC hDC = ::GetDC(hWnd);
			//_hDC = ::CreateCompatibleDC(hDC);
			//::ReleaseDC(hWnd, hDC);
			_hDC = ::CreateCompatibleDC(NULL);
			_hBitmap = ::CreateBitmap(128, 128, 1, 32, NULL);//::CreateCompatibleBitmap(_hDC, 128, 128);
			E_ASSERT(_hBitmap != NULL);
			E_ASSERT(_hDC != NULL);
			HGDIOBJ h =::SelectObject(_hDC, _hBitmap);
			E_ASSERT(h != NULL);
#endif

#ifdef NB_LINUX
			if(_display == 0)
			{
				if((_display = XOpenDisplay(NULL)) == NULL)
				{
					E_ASSERT1(0, "[nb] Failed to open display.");
					write_log("[nb] (EE) Failed to open display.");
					exit(1);
				}
				_screenNum = DefaultScreen(_display);
				_screenColormap = DefaultColormap(_display, DefaultScreen(_display));
			}
			blackColor.red   = 0;
			blackColor.green = 0;
			blackColor.blue  = 0;
			::XAllocColor(_display, _screenColormap, &blackColor);
			whiteColor.red = 0xffff;
			whiteColor.green = 0xffff;
			whiteColor.blue = 0xffff;
			::XAllocColor(_display, _screenColormap, &whiteColor);
			xftColor.color.red = 0;
			xftColor.color.green = 0;
			xftColor.color.blue = 0;
			xftColor.pixel = blackColor.pixel;
			xftColor.color.alpha = 0x00ff00;
			image = 0;
			depth = DefaultDepth(_display, _screenNum);

			if(depth <= 16 && depth>=0)
			{
				local_palette_size = 0x00000001 << depth;
				local_palette = (Palette*)malloc(sizeof(Palette) * local_palette_size);
				memset(local_palette, 0, sizeof(Palette) * local_palette_size);
			}

			Window w = RootWindow(_display, _screenNum);
			drawable = ::XCreatePixmap(_display, w, 128, 128, depth);
			E_ASSERT(drawable);
			unsigned long valuemask = 0;
			XGCValues values;
			gc = ::XCreateGC(_display, drawable, valuemask, &values);
			E_ASSERT(gc);
			xft = ::XftDrawCreate(_display, drawable, DefaultVisual(_display, _screenNum), _screenColormap);
			// TODO: switch to XftDrawCreateAlpha()?
			E_ASSERT(xft);
#endif
		}
		_refCount++;

		nativeFont = CreateNativeFont(_fontName, _fontSize, _bold);
		E_ASSERT(nativeFont); // TODO: exception handling. create native font will never fail, but, who know...
	}

	SystemFontLoader::~SystemFontLoader()
	{
		delete imp2;
	}

	SystemFontLoaderImp::~SystemFontLoaderImp()
	{
#ifdef NB_WINDOWS
		::DeleteObject(nativeFont);
#endif

#ifdef NB_LINUX
		::XftFontClose(_display, nativeFont);
#endif
		_refCount--;
		if(_refCount == 0)
		{
#ifdef NB_WINDOWS
			::DeleteDC(_hDC);
			_hDC = 0;
			::DeleteObject(_hBitmap);
			_hBitmap = 0;
#endif

#ifdef NB_LINUX
			free(local_palette);
			local_palette = 0;
			if(image)
			{
				XDestroyImage(image);
				image = 0;
			}
			::XftDrawDestroy(xft);
			xft = 0;
			::XFreeGC(_display, gc);
			gc = 0;
			::XFreePixmap(_display, drawable);
			drawable = 0;
#endif
//			E_TRACE_LINE("[nb] Free drawing buffer.");
		}
	}

	inline static void LimitWH(int & _w, int & _h)
	{
		if(_w < 1)
		{
			_w = 1;
		}

		if(_w > Font::MAX_FONT_SIZE)
		{
			_w = Font::MAX_FONT_SIZE;
		}
		if(_h < 1)
		{
			_h = 1;
		}

		if(_h > Font::MAX_FONT_SIZE)
		{
			_h = Font::MAX_FONT_SIZE;
		}
	}

	bool SystemFontLoader::LoadGlyph(Char _ch, Image & _pic)
	{
		return imp2->LoadGlyph(_ch, _pic);
	}

	bool SystemFontLoaderImp::LoadGlyph(Char _ch, Image & _pic)
	{
		//_buf[0] = 0;
		Char buf[2];
		buf[0] = _ch;
		buf[1] = 0;
#ifdef NB_WINDOWS
		HFONT hOldFont = (HFONT)::SelectObject(_hDC, nativeFont);
		static RECT _rect = {0, 0, 100, 100};
		::FillRect(_hDC, &_rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));
		::TextOut(_hDC, 0, 0, buf, 1);
		SIZE sz;
		::GetTextExtentPoint32(_hDC, buf, 1, &sz);
		::SelectObject(_hDC, hOldFont);
		int w = sz.cx;
		int h = sz.cy;
		LimitWH(w, h);
		_pic.Alloc(w, h);
		_pic.Fill(0x00ffffff);
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x ++)
			{
				COLORREF c = ::GetPixel(_hDC, x, y);
				int i = GetRValue(c) + GetGValue(c) + GetBValue(c);
				if(i < 765)
				{
					uint8 a = (uint8)(255 - i / 3);
					uint8 * p = _pic.Get(x, y);
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
					p[3] = a;
//E_ASSERT(a == 255);
				}
			}
		}
		//_pic.Save(Env::GetDataFolder() | string(_ch) + L"system.png", Image::PNG);
#endif

#ifdef NB_LINUX

#ifdef UNICODE
#	ifdef __CYGWIN__
#		define XftTextExtentsX XftTextExtents16
#		define XftDrawStringX XftDrawString16
#		define FcCharX FcChar16
#	else
#		define XftTextExtentsX XftTextExtents32
#		define XftDrawStringX XftDrawString32
#		define FcCharX FcChar32
#	endif
#else
#	define XftTextExtentsX XftTextExtents8
#	define XftDrawStringX XftDrawString8
#	define FcCharX FcChar8
#endif

		if(image)
		{
			XDestroyImage(image);
			image = 0;
		}
		XGlyphInfo info;
		XftTextExtentsX(_display, nativeFont, (const FcCharX *)(const wchar_t *)buf, 1, &info);
		int w = info.xOff;
		int h = nativeFont->height;
		int x = 0;
		int y = h;

		::XSetForeground(_display, gc, whiteColor.pixel);
		::XFillRectangle(_display, drawable, gc,  0, 0, w, h);
		//::XDrawRectangle(_display, drawable, gc, 0, 0, _w, _h);
		::XftDrawStringX(xft, &xftColor, nativeFont, x, y  - nativeFont->descent,
			(const FcCharX *)(const char *)buf, 1);
		image = ::XGetImage(_display, drawable, 0, 0, w, h, AllPlanes,  XYPixmap );
		LimitWH(w, h);
		_pic.Alloc(w, h);
		_pic.Fill(0x00ffffff);


#	if 1
		// optimize for true color
		if(depth == 24 || depth == 32)
		{
			// optimize for true color
			for(int y = 0; y < h; y++)
			{
				for(int x = 0; x < w; x ++)
				{
					unsigned long pixel = XGetPixel(image, x, y);
					int r = pixel & 0xff;
					int g = (pixel >> 8) & 0xff;
					int b = (pixel >> 16) & 0xff;
					int a = 255 - ((r+g+b) / 3);
					if(a)
					{
						uint8 * p = _pic.Get(x, y);
						p[0] = 255;
						p[1] = 255;
						p[2] = 255;
						p[3] = a;
					}
				}
			}
		}
		else
#	endif
		{
			for(int y = 0; y < h; y++)
			{
				for(int x = 0; x < w; x ++)
				{
					XColor c;
					c.pixel = XGetPixel(image, x, y);
					uint8 a;

					if(local_palette && c.pixel < local_palette_size)
					{
						Palette & pal = local_palette[c.pixel];
						if(pal[1] == 0)
						{
							XQueryColor(_display, _screenColormap, &c);
							int i = c.red + c.green + c.blue;
							pal[0] = (uint8)(255 - i / 3);
						}
						a = pal[0];
					}
					else
					{
						XQueryColor(_display, _screenColormap, &c);
						int i = c.red + c.green + c.blue;
						a = (uint8)(255 - i / 3);
					}

					if(a)
					{
						uint8 * p = _pic.Get(x, y);
						p[0] = 255;
						p[1] = 255;
						p[2] = 255;
						p[3] = a;
					}
				}
			}
		}
#endif

		return true;
	}
}
