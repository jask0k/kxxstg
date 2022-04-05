#include <nbug/ui/themes.h>
#include <nbug/gl/graphics.h>
#include <nbug/core/env.h>

namespace e
{
	static void DrawRect(Graphics * _g, float _x0, float _y0, float _x1, float _y1, const RGBA & _fill, const RGBA & _border)
	{
		_g->BindTex(0);
		_g->SetColor(_fill);
		_g->DrawQuad(_x0, _y0, _x1, _y1);

		_g->SetColor(_border);
		Vector3 pts[5] = 
		{
			{_x0, _y0, 0},
			{_x1, _y0, 0},
			{_x1, _y1, 0},
			{_x0, _y1, 0},
			{_x0, _y0, 0},
		};
		_g->SetVertexSource(pts, sizeof(Vector3), V::FORMAT, 5);
		_g->DrawPrimitive(E_LINESTRIP, 0, 5);
	}

	Themes::Themes(Graphics * _g)
	{
		itemMargin = 2;
		controlMargin = 2;
		iconSize = 20;
		controlColor.SetRgbaDword(0xffaaaaaa);

		E_ASSERT(_g);
		g = _g;
		font = g->LoadFont(L"Tahoma", 12);

		buttonTex = g->LoadTexFromFile(Env::GetResourceFolder() | L"ui_button");

		int tw = buttonTex->W();
		int th = buttonTex->H();
		int th1 = (th - 2) / 3;

		btntcx[0] = 0;
		btntcx[1] = 3.0f / tw;
		btntcx[2] = (tw - 3.0f) / tw;
		btntcx[3] = 1.0f;

		for(int i=0; i < 3; i++)
		{
			float y0 = (float)(i * th1 + i);
			btntcy[i][0] = y0 / th;
			btntcy[i][1] = (y0 + 3.0f)/ th;
			btntcy[i][2] = (y0 + th1 - 3.0f) / th;
			btntcy[i][3] = (y0 + th1) / th;
		}

	}

	Themes::~Themes()
	{
	}

	void Themes::DrawButton(float _w, float _h, float _push, float _hover)
	{
		//RGBA fill = {1-_hover, 1, 1, 1};
		//RGBA border = {0, 0, 0, 1};
		//DrawRect(g, 0, 0, _w, _h, fill, border);
		g->BindTex(buttonTex);
		g->SetTexMode(TM_REPLACE);
		//g->SetColor(1, 1, 1, 1);
		g->DrawQuad(0, 0, 3, 3,       btntcx[0], btntcy[0][0], btntcx[1], btntcy[0][1]);
		g->DrawQuad(0, 3, 3, _h - 3,  btntcx[0], btntcy[0][1], btntcx[1], btntcy[0][2]);
		g->DrawQuad(0, _h - 3, 3, _h, btntcx[0], btntcy[0][2], btntcx[1], btntcy[0][3]);

		g->DrawQuad(3, 0, _w - 3, 3,       btntcx[1], btntcy[0][0], btntcx[2], btntcy[0][1]);
		g->DrawQuad(3, 3, _w - 3, _h - 3,  btntcx[1], btntcy[0][1], btntcx[2], btntcy[0][2]);
		g->DrawQuad(3, _h - 3, _w - 3, _h, btntcx[1], btntcy[0][2], btntcx[2], btntcy[0][3]);
		g->DrawQuad(_w - 3, 0, _w, 3,        btntcx[2], btntcy[0][0], btntcx[3], btntcy[0][1]);
		g->DrawQuad(_w - 3, 3, _w, _h - 3,   btntcx[2], btntcy[0][1], btntcx[3], btntcy[0][2]);
		g->DrawQuad(_w - 3, _h - 3, _w , _h, btntcx[2], btntcy[0][2], btntcx[3], btntcy[0][3]);

		if(_push > 0 )
		{
			g->SetTexMode(TM_MODULATE);
			g->SetColor(1, 1, 1, _push);
			g->DrawQuad(0, 0, 3, 3,       btntcx[0], btntcy[1][0], btntcx[1], btntcy[1][1]);
			g->DrawQuad(0, 3, 3, _h - 3,  btntcx[0], btntcy[1][1], btntcx[1], btntcy[1][2]);
			g->DrawQuad(0, _h - 3, 3, _h, btntcx[0], btntcy[1][2], btntcx[1], btntcy[1][3]);

			g->DrawQuad(3, 0, _w - 3, 3,       btntcx[1], btntcy[1][0], btntcx[2], btntcy[1][1]);
			g->DrawQuad(3, 3, _w - 3, _h - 3,  btntcx[1], btntcy[1][1], btntcx[2], btntcy[1][2]);
			g->DrawQuad(3, _h - 3, _w - 3, _h, btntcx[1], btntcy[1][2], btntcx[2], btntcy[1][3]);
			g->DrawQuad(_w - 3, 0, _w, 3,        btntcx[2], btntcy[1][0], btntcx[3], btntcy[1][1]);
			g->DrawQuad(_w - 3, 3, _w, _h - 3,   btntcx[2], btntcy[1][1], btntcx[3], btntcy[1][2]);
			g->DrawQuad(_w - 3, _h - 3, _w , _h, btntcx[2], btntcy[1][2], btntcx[3], btntcy[1][3]);
		}

		if(_hover > 0)
		{
			g->SetTexMode(TM_MODULATE);
			g->SetBlendMode(BM_ADD);
			g->SetColor(1, 1, 1, _hover * 0.2f);
			g->DrawQuad(0, 0, 3, 3,       btntcx[0], btntcy[2][0], btntcx[1], btntcy[2][1]);
			g->DrawQuad(0, 3, 3, _h - 3,  btntcx[0], btntcy[2][1], btntcx[1], btntcy[2][2]);
			g->DrawQuad(0, _h - 3, 3, _h, btntcx[0], btntcy[2][2], btntcx[1], btntcy[2][3]);

			g->DrawQuad(3, 0, _w - 3, 3,       btntcx[1], btntcy[2][0], btntcx[2], btntcy[2][1]);
			g->DrawQuad(3, 3, _w - 3, _h - 3,  btntcx[1], btntcy[2][1], btntcx[2], btntcy[2][2]);
			g->DrawQuad(3, _h - 3, _w - 3, _h, btntcx[1], btntcy[2][2], btntcx[2], btntcy[2][3]);
			g->DrawQuad(_w - 3, 0, _w, 3,        btntcx[2], btntcy[2][0], btntcx[3], btntcy[2][1]);
			g->DrawQuad(_w - 3, 3, _w, _h - 3,   btntcx[2], btntcy[2][1], btntcx[3], btntcy[2][2]);
			g->DrawQuad(_w - 3, _h - 3, _w , _h, btntcx[2], btntcy[2][2], btntcx[3], btntcy[2][3]);
			g->SetBlendMode(BM_NORMAL);
		}
	}

	void Themes::DrawLabel(float _w, float _h, const string & _text)
	{
		g->SetTexMode(TM_MODULATE);
		g->SetFont(font);
		g->SetColor(0, 0, 0, 1);
		g->DrawString(4, 4, _w - 4, _h - 4, _text);
	}

	void Themes::GetTextExt(const string & _s, float & _w, float & _h)
	{
		_w = (float)font->W(_s.c_str(), _s.length());
		_h = (float)font->H();
	}

}
