
// #include "../config.h"
#include <z_kxx/sprite/corpse.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{

	bool Corpse::Step()
	{
		if(--timer == 0)
		{
			return false;
		}

		clr.a+= alpha_delta;
		clr.g = clr.b = clr.a;
		scl.x+=scale_delta;
		scl.y+=scale_delta;
		pos+= vel;
		vel+= acc;
		ang+= rot;
		return true;
	}

	void Corpse::Render()
	{
		graphics->SetColor(clr);
		graphics->SetBlendMode(BM_NORMAL);

		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);
		if(ang)
		{
			graphics->RotateMatrix(ang, 0, 0, 1);
		}
		float scaledHalfWidth = hsz.x * scl.x;
		float scaledHalfHeight = hsz.y * scl.y;
		graphics->BindTex(tex);
		graphics->DrawQuad(-scaledHalfWidth, -scaledHalfHeight, scaledHalfWidth, scaledHalfHeight,
			tx0, ty0, tx1, ty1);
		graphics->PopMatrix();
	}

}
