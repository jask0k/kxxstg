
// #include "../config.h"
#include <z_kxx/sprite/dynamic_pic.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	DynamicImage::DynamicImage(const stringa & _name, float _x0, float _y0, float _x1, float _y1)
	{
		tex = kxxwin->LoadTex(_name);
		bound.x = _x0;
		bound.y = _y0;
		bound.w = _x1 - _x0;
		bound.h = _y1 - _y0;	
		autoMode = true;
		timerMax[0] = timer[0] = 60;
		timerMax[1] = timer[1] = 180;
		timerMax[2] = timer[2] = 60;
		state = 0;
		method = Alpha;
	}

	DynamicImage::~DynamicImage()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void DynamicImage::Render()
	{
		if(!tex)
		{
			return;
		}

		switch(method)
		{
		case Alpha:
			// graphics->BlendOn();
			graphics->SetTexMode(TM_MODULATE);
			// graphics->SetTexMode(TextureMode::Modulate);
			graphics->BindTex(tex);
			graphics->SetColor(1, 1, 1, GetAlpha());
			graphics->DrawQuad(bound.L(), bound.T(), bound.R(), bound.B());
			// graphics->SetTexMode(TextureMode::replace);
			break;
		case FlipV:
			{
				float a = GetAlpha();
				float h = bound.h * 0.5f * a;
				float c = (bound.B() + bound.T()) * 0.5f;
				// graphics->BlendOn();
				graphics->SetTexMode(TM_MODULATE);
				//// graphics->SetTexMode(TextureMode::Modulate);
				graphics->BindTex(tex);
				//graphics->SetColor(1, 1, 1, GetAlpha());
				graphics->DrawQuad(bound.L(), c-h, bound.R(), c+h);
				//// graphics->SetTexMode(TextureMode::replace);
			}
			break;
		case FlipH:
			{
				float a = GetAlpha();
				float w = bound.w * 0.5f * a;
				float c = (bound.L() + bound.R()) * 0.5f;
				// graphics->BlendOn();
				graphics->SetTexMode(TM_MODULATE);
				//// graphics->SetTexMode(TextureMode::Modulate);
				graphics->BindTex(tex);
				//graphics->SetColor(1, 1, 1, GetAlpha());
				graphics->DrawQuad(c-w, bound.T(), c+w, bound.B());
				//// graphics->SetTexMode(TextureMode::replace);
			}
			break;
		}
	}

	bool DynamicImage::Step()
	{
		if(state >= 3 || state == 1 && !autoMode)
		{
			return true;
		}

		if(timer[state])
		{
			timer[state]--;
		}
		else
		{
			state++;
			if(state>=3)
			{
				return false;
			}
		}

		return true;
	}

	void DynamicImage::FadeOut()
	{
		state = 2;
	}
	
	void DynamicImage::SetTimer(uint32 _in, uint32 _keep, uint32 _out, bool _auto)
	{
		timerMax[0] = timer[0] = _in;
		timerMax[1] = timer[1] = _keep;
		timerMax[2] = timer[2] = _out;
		autoMode = _auto;
	}
}
