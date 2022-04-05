
#include <z_kxx/sprite/sc_name.h>
#include <z_kxx/sprite/spark1.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
#define SC_NAME_MARGIN 20
#define SC_NAME_SCALE 1.5f
#define SC_NAME_SCALE_FADE_IN 5.0f
#define SC_NAME_STATE_1_TIMER (20 * K_LOGIC_FPS_MUL)
#define SC_NAME_STATE_2_TIMER (60 * K_LOGIC_FPS_MUL)
#define SC_NAME_STATE_3_TIMER (60 * K_LOGIC_FPS_MUL)

	SCName::SCName(bool _is_boss, TexRef & _bg, FontRef & _font)
	{
		is_boss = _is_boss;
		state = 0;
		font = _font;
		tex = _bg;
	}

	void SCName::Start(const string & _name)
	{
		name = _name;

		text_w = (float)font->W(name.c_str(), name.length());
		text_h = (float)font->H();
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;
		text_org.y = -text_h * 0.5f;
		Vector2 v;
		if(is_boss)
		{
			v.x = SC_NAME_MARGIN + tex->W() * 0.5f;
			v.y = 30;

			pos.x = SC_NAME_MARGIN + SC_NAME_SCALE * tex->W() * 0.5f;
			pos.y = K_GAME_YC - 30;

			text_clr.r = 0.8f;
			text_clr.g = 0.8f;
			text_clr.b = 0.8f;
			text_org.x = -hsz.x + 8;

		}
		else
		{
			v.x = K_GAME_W - SC_NAME_MARGIN - tex->W() * 0.5f;
			v.y = K_GAME_H - 50;

			pos.x = K_GAME_W - SC_NAME_MARGIN - tex->W() * SC_NAME_SCALE * 0.5f;
			pos.y = K_GAME_YC + 30;

			text_clr.r = 0.8f;
			text_clr.g = 0.8f;
			text_clr.b = 0.8f;
			text_org.x = hsz.x - text_w - 8;
		}
		state3_vel.x = (v.x - pos.x) / SC_NAME_STATE_3_TIMER;
		state3_vel.y = (v.y - pos.y) / SC_NAME_STATE_3_TIMER;
		scl.x = SC_NAME_SCALE + SC_NAME_SCALE_FADE_IN;
		scl.y = 0;
		clr.a = 0;
		text_clr.a = 0;
		state = 1;
		timer = SC_NAME_STATE_1_TIMER;
	}

	bool SCName::Step()
	{
		switch(state)
		{
		case 0:
			break;
		case 1:
			// show
			if(--timer == 0)
			{
				state = 2;
				timer = SC_NAME_STATE_2_TIMER;
			}
			else
			{
				scl.x-= SC_NAME_SCALE_FADE_IN/ SC_NAME_STATE_1_TIMER;
				scl.y+= SC_NAME_SCALE * 1.0f / SC_NAME_STATE_1_TIMER;
				clr.a+= 0.99f / SC_NAME_STATE_1_TIMER;
				text_clr.a = clr.a;
			}
			break;
		case 2:
			// stick
			if(--timer == 0)
			{
				state = 3;
				timer = SC_NAME_STATE_3_TIMER;
			}
			break;
		case 3:
			// move to coner
			if(--timer == 0)
			{
				state = 0;
			}
			else
			{
				scl.x-= (SC_NAME_SCALE - 1.0f) / SC_NAME_STATE_3_TIMER;
				scl.y-= (SC_NAME_SCALE - 1.0f) / SC_NAME_STATE_3_TIMER;
				pos+= state3_vel;
				clr.a-= 0.50f / SC_NAME_STATE_3_TIMER;
				text_clr.a = clr.a;
			}
			break;
		}
		return true;
	}

	void SCName::Render()
	{
		graphics->SetColor(clr);
		graphics->SetBlendMode(blm);

		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);
		graphics->ScaleMatrix(scl.x, scl.y, 0);
		graphics->BindTex(tex);
		float scaledHalfWidth = hsz.x;
		float scaledHalfHeight = hsz.y;
		graphics->DrawQuad(-scaledHalfWidth, -scaledHalfHeight, scaledHalfWidth, scaledHalfHeight);


		graphics->SetColor(text_clr);
		graphics->SetFont(font);
		graphics->DrawString(text_org.x, text_org.y, name);
		graphics->PopMatrix();
	}

	void SCName::Explode()
	{
		Vector2 v;
		for(int i=0; i<20; i++)
		{
			v.x = pos.x + (frand() - 0.5f) * 2 * hsz.x * scl.x;
			v.y = pos.y + (frand() - 0.5f) * 2 * hsz.y * scl.y;
			kxxwin->AddSmallSpark(v, 1, 0);
		}
	}

}