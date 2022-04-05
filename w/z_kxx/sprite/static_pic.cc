
// #include "../config.h"
#include <z_kxx/sprite/static_pic.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	StaticImage::StaticImage(const stringa & _name, float _x0, float _y0, float _x1, float _y1)
	{
		tex = kxxwin->LoadTex(_name);
		bound.x = _x0;
		bound.y = _y0;
		bound.w = _x1 - _x0;
		bound.h = _y1 - _y0;	
	}

	StaticImage::~StaticImage()
	{
	}

	void StaticImage::Render()
	{
		if(!tex)
		{
			return;
		}
		graphics->SetBlendMode(BM_NORMAL);
		graphics->SetTexMode(TM_MODULATE);
		graphics->SetColor(clr);
		graphics->BindTex(tex);
		graphics->DrawQuad(bound.L(), bound.T(), bound.R(), bound.B());
	}
	
}
