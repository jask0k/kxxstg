
// #include "../config.h"
#include <z_kxx/effect/zizo_fade.h>

namespace e
{
	ZIZOFade::ZIZOFade(bool _x, bool _y)
		: Fade(true, true)
	{
		zoomX = _x;
		zoomY = _y;
		overlay= false;
	}


	ZIZOFade::~ZIZOFade()
	{
	}
	
	void ZIZOFade::Render()
	{
		if(!fboA || !fboB)
		{
			return;
		}
		float frac = GetFrac();
		float xc = w * 0.5f;
		float yc = h * 0.5f;
#define S 0.33f
		bool is_a = frac <= S;
		frac = (is_a) ? (S - frac) / S : (frac - S) /(1.0f-S);
		frac = frac * 0.5f;
		float w1 = frac * w;
		float h1 = frac * h;
		float x0 = zoomX ? xc - w1 : 0;
		float x1 = zoomX ? xc + w1 : w;
		float y0 = zoomY ? yc - h1 : 0;
		float y1 = zoomY ? yc + h1 : h;
		g->SetColor(0xFFFFFFFF);
		g->SetClearColor(0, 0,0, 1);
		g->ClearBuffer(true, false, false);
		// g->BlendOff();
		g->SetTexMode(TM_MODULATE);
		g->BindTex(is_a ? fboA->GetTex() : fboB->GetTex());
		g->DrawQuad(x0, y0, x1, y1);
		// g->BlendOn();
	}
}
