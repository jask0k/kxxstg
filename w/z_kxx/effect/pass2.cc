
// #include "../config.h"
#include <nbug/gl/graphics.h>
#include <z_kxx/effect/pass2.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	Pass2::Pass2()
	{
		shader = 0;
		explode_n = 0;
	}

	Pass2::~Pass2()
	{
	}

	bool Pass2::Init(Graphics * _g, float _w, float _h)
	{
		g = _g;
		return false;
		if(_g->GetDeviceType() != Graphics::OpenGL)
		{
			return false;
		}

		w = _w;
		h = _h;
		shader = _g->LoadShader(
			0,
			kxxwin->ResFolder() | L"shader" | L"pass2_pixel.glsl");

		if(!shader)
		{
			return false;
		}

		fbo = _g->CreateFbo((int)w, (int)h);
		if(!fbo)
		{
			return false;
		}

		explode_n_loc = shader->GetUniformLoc("explode_n");
		explode_loc = shader->GetUniformLoc("explode");
		y_scale_loc = shader->GetUniformLoc("y_scale");

		return true;
	}

	static const int EXPLODE_TIMER0 = 30;
	static const int EXPLODE_TIMER1 = 15;

	void Pass2::AddExplode(float _x, float _y)
	{
		if(explode_n < 10)
		{
			explode_timer[explode_n] = EXPLODE_TIMER0;
			explode[explode_n].x = _x / w;
			explode[explode_n].y = (h - _y) / h;
			explode[explode_n].z = 0.0f;
			explode[explode_n].w = 1.0f;
			explode_n++;
		}
	}

	void Pass2::Step()
	{
		for(int i=0; i<explode_n;)
		{
			uint32 &t = explode_timer[i];
			if(--t == 0)
			{
				if(i < explode_n)
				{
					memmove(explode_timer + i, explode_timer + i + 1, sizeof(uint32) * (explode_n - i ));
					memmove(explode + i, explode + i + 1, sizeof(Vector4) * (explode_n - i));
				}
				explode_n--;
				//E_TRACE_LINE(string(explode_n));
			}
			else
			{
				float a = (float(t) - float(EXPLODE_TIMER1)) / float(EXPLODE_TIMER1);
				a = fabs(a);
				//float b = (float(EXPLODE_TIMER0) - float(t)) / float(EXPLODE_TIMER0);

				explode[i].z =  (1.0f-a) * 0.10f;
				explode[i].w =  a * 0.3f + 0.7f;
				i++;
			}
		}
	}

	void Pass2::Render()
	{
		g->SetBlendMode(BM_DISABLE);
		g->SetTexMode(TM_REPLACE);
		graphics->BindTex(fbo->GetTex());
		graphics->SetShader(shader);
		graphics->SetUniform(y_scale_loc, h/w);
		graphics->SetUniform(explode_n_loc, explode_n);
		graphics->SetUniform(explode_loc, explode, explode_n);
		graphics->DrawQuad(0, 0, w, h);
		graphics->SetShader(0);
		g->BindTex(0);
		g->SetBlendMode(BM_NORMAL);
		g->SetTexMode(TM_MODULATE);
	}

}
