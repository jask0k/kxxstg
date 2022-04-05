#pragma once

#include <nbug/gl/tex.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/color.h>

namespace e
{
	class Graphics;
	class Themes
	{
		Graphics * g;
		TexRef buttonTex;
		FontRef font;
		float btntcx[4];
		float btntcy[3][4];
	public:
		float itemMargin;
		float iconSize;
		float controlMargin;
		RGBA controlColor;
		Themes(Graphics * _g);
		Graphics * GetGraphics()
		{ return g; }
		virtual ~Themes();
		void DrawButton(float _w, float _h, float _push, float _hover);
		void DrawLabel(float _w, float _h, const string & _text);
		void GetTextExt(const string & _s, float & _w, float & _h);
	};

}

