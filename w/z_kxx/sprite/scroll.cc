#include <z_kxx/sprite/scroll.h>

namespace e
{
//#define X_MARGIN 20.0f
//#define Y_MARGIN 30.0f
#define X_TO_BORDER 40.0f
#define Y_TO_BORDER 60.0f
#define ROLLER_R 8.0f
#define STATE0_TIMER int(0.4f * K_LOGIC_FPS)
#define STATE1_TIMER int(0.8f * K_LOGIC_FPS)
#define STATE2_TIMER int(2.5f * K_LOGIC_FPS)

	static float len_to_roller1_y0(float _len)
	{ return 2 * PI  * ROLLER_R + _len; }

	Scroll::Scroll(TexRef & _fgTex, TexRef & _bgTex)
	{
		state = 0;
		timer = S2F(1.0f);
		tex = _fgTex;
		bg_tex = _bgTex;
		offset = 0;
		len = 0;
		
		bg_h = (float)bg_tex->H();
		bg_w = (float)bg_tex->W();
		fg_h = (float)tex->H();
		fg_w = (float)tex->W();
		fg_x = (bg_w - fg_w) * 0.5f;
		fg_y = (bg_h - fg_h) * 0.5f;

		max_len = bg_h - 3.0f * ROLLER_R * PI;

		hsz.x = 0.5f * bg_w;

		hsz.y = max_len + ROLLER_R;
		pos.x = K_GAME_W + hsz.x;
		pos.y = 80;
		float da = PI *0.1f;
		float a = PI;
		RGBA rgba;
		rgba.a = 1.0f;
		for(int i=0; i<10; i++)
		{
			float y = ROLLER_R * (1.0f + cos(a)) - ROLLER_R;
			float cf = 1.0f - fabs((i-5.0f)) * 0.2f;
			rgba.r = 0.2f + cf * 0.6f;
			rgba.g = 0.2f + cf * 0.7f;
			rgba.b = 0.2f + cf * 0.8f;
			InternalColor c = graphics->RGBAtoInternalColor(rgba);
			roller0[i*2+0].v.x = -hsz.x; 
			roller0[i*2+0].v.y = y; 
			roller0[i*2+0].v.z = 0; 
			roller0[i*2+0].c = c;
			roller0[i*2+1].v.x = +hsz.x; 
			roller0[i*2+1].v.y = y; 
			roller0[i*2+1].v.z = 0; 
			roller0[i*2+1].c = c;
			a+= da;
		}

		memcpy(roller1, roller0, sizeof(roller0));

		for(int i=0; i<10; i++)
		{
			float y = (ROLLER_R * (1.0f + cos(a)) - ROLLER_R) * 0.6f;
			float cf = 1.0f - fabs((i-5.0f)) * 0.2f;
			rgba.r = 0.1f + cf * 0.6f;
			rgba.g = 0.1f + cf * 0.2f;
			rgba.b = 0.1f + cf * 0.2f;
			InternalColor c = graphics->RGBAtoInternalColor(rgba);
			roller2[i*2+0].v.x = -hsz.x - 8; 
			roller2[i*2+0].v.y = y; 
			roller2[i*2+0].v.z = 0; 
			roller2[i*2+0].c = c;
			roller2[i*2+1].v.x = +hsz.x + 8; 
			roller2[i*2+1].v.y = y; 
			roller2[i*2+1].v.z = 0; 
			roller2[i*2+1].c = c;
			a+= da;
		}

		memcpy(roller3, roller2, sizeof(roller2));

		UpdateRoller(roller0, (ROLLER_R * PI)/ bg_tex->H(), 0);
		float y0 = len_to_roller1_y0(len);
		UpdateRoller(roller1, y0/ bg_tex->H(), (y0 + ROLLER_R * PI)/ bg_h);
		state = 0;
		timer = STATE0_TIMER;
		delta = (X_TO_BORDER + hsz.x * 2) / timer;
	}

	Scroll::~Scroll()
	{
	}

	void Scroll::Render()
	{
		float ty0 = (offset + 1.5f*PI * ROLLER_R) / bg_h;
		float ty1 = ty0 + len /  bg_h;

		graphics->SetColor(clr);
		graphics->SetBlendMode(blm);
		//graphics->SetTexMode(TM_REPLACE);

		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);

		float hw = hsz.x;
		float hh = hsz.y;

		graphics->BindTex(bg_tex);
		float x0 = -hw;
		float x1 = +hw;
		float y0 = -len * 0.5f;
		float y1 = +len * 0.5f;
		graphics->DrawQuad(
			x0,y0, 0, 0, ty0,
			x1,y0, 0, 1, ty0,
			x1,y1, 0, 1, ty1,
			x0,y1, 0, 0, ty1
			);

		if(len > ROLLER_R * 1.8f)
		{
			float x00 = x0 + fg_x;
			float x11 = x00 + fg_w;

			float y00 = y0 -offset - 1.5f*PI * ROLLER_R + fg_y;
			float y11 = fg_h + y00;

			if(y00 < y0)
			{
				ty0 = (y0 - y00) / fg_h;
				y00 = y0;
			}
			else
			{
				ty0 = 0;
			}

			if(y11 > y1)
			{
				ty1 = 1.0f - (y11 - y1) / fg_h;
				y11 = y1;
			}
			else
			{
				ty1 = 1.0f;
			}

			if(y00 < y11)
			{
				graphics->BindTex(tex);
				graphics->DrawQuad(
					x00,y00, 0, 0, ty0,
					x11,y00, 0, 1, ty0,
					x11,y11, 0, 1, ty1,
					x00,y11, 0, 0, ty1
					);

				graphics->BindTex(bg_tex);
			}
		}

		graphics->TranslateMatrix(0, -len*0.5f, 0);

		graphics->SetTexMode(TM_DISABLE);
		graphics->SetVertexSource(roller2, sizeof(VC), VC::FORMAT, 20);
		graphics->DrawPrimitive(E_TRIANGLESTRIP, 0, 20);

		graphics->SetTexMode(TM_MODULATE);
		graphics->SetVertexSource(roller0, sizeof(VCT), VCT::FORMAT, 20);
		graphics->DrawPrimitive(E_TRIANGLESTRIP, 0, 20);


		graphics->TranslateMatrix(0, len, 0);

		graphics->SetTexMode(TM_DISABLE);
		graphics->SetVertexSource(roller3, sizeof(VC), VC::FORMAT, 20);
		graphics->DrawPrimitive(E_TRIANGLESTRIP, 0, 20);

		graphics->SetTexMode(TM_MODULATE);
		graphics->SetVertexSource(roller1, sizeof(VCT), VCT::FORMAT, 20);
		graphics->DrawPrimitive(E_TRIANGLESTRIP, 0, 20);

		graphics->PopMatrix();

	}

	bool Scroll::Step()
	{
		switch(state)
		{
		case 0:
			if(--timer == 0)
			{
				state = 1;
				timer = STATE1_TIMER;
				delta = max_len / timer;
			}
			else
			{
				pos.x-= delta;
			}
			break;
		case 1:
			// open
			if(--timer == 0)
			{
				state = 2;
				timer = STATE2_TIMER;
			}
			else
			{
				len+= delta;
				pos.y+= delta * 0.5f;
				float y0 = len_to_roller1_y0(len);
				UpdateRoller(roller1, y0 /  bg_h, (y0 + ROLLER_R * PI)/ bg_h);
			}
			break;
		case 2:
			// show
			if(--timer == 0)
			{
				state = 3;
				timer = STATE1_TIMER;
				delta = max_len / timer;
			}
			else
			{
			}
			break;
		case 3:
			// close
			if(--timer == 0)
			{
				state = 4;
				timer = STATE0_TIMER;
				delta = (X_TO_BORDER + hsz.x * 2) / timer;
			}
			else
			{
				offset+= delta;
				len-= delta;
				pos.y+= delta * 0.5f;
				UpdateRoller(roller0, (offset + ROLLER_R * PI)/ bg_h, offset /  bg_h);
			}
			break;
		case 4:
			if(--timer == 0)
			{
				state = 5;
				timer = STATE0_TIMER;
			}
			else
			{
				pos.x+= delta;
			}
			break;
		}
		return state < 5;
	}

	void Scroll::UpdateRoller(VCT v[], float _ty0, float _ty1)
	{
		float dy = (_ty1 - _ty0) * 0.1f;
		float y = _ty0;
		for(int i=0; i<10; i++)
		{
			E_ASSERT(y <= 1.0f);
			v[i*2+0].t.x = 0.0f; 
			v[i*2+0].t.y = y; 
			v[i*2+1].t.x = 1.0f; 
			v[i*2+1].t.y = y; 
			y+= dy;
		}
	}

}
