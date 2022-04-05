#pragma once

#include <z_kxx/shot/shot.h>

#ifdef NB_DEBUG

namespace e
{
	class DebugShot : public EnemyShot
	{
		Vector2 c;
		float r;
		float a;
		string name;
	public:
		DebugShot(int _type, int _color, const Vector2 & _v, float _angle)
		{
			c = _v;
			a = _angle;
			r = 40;
			name = string(_type);
			name.append((wchar_t)(L'a' + _color));
			if(graphics->GetTexPool()->GetIndexer()->GetPath(L"shot-" + name).IsValid())
			{
				tex = kxxwin->shotTex[_type][_color];
				hsz.x = tex->W() * 0.5f;
				hsz.y = tex->H() * 0.5f;
			}
			else
			{
				tex = 0;
				hsz.x = 10;
				hsz.y = 10;
			}
			pos = _v;
		}

		bool Step() override
		{
			if(r < 90)
			{
				r+= 0.07f;
			}
			else if(r < 140)
			{
				r+= 0.04f;
				a+= 0.003f;
				ang = a + PI;
			}
			else if(r < 190)
			{
				r+= 0.04f;
				ang+= 0.004f;
			}
			else
			{
				r+= 0.05f;
				ang = a + PI * 0.5f;
			}
			pos.x = c.x + r * cos(a);
			pos.y = c.y + r * sin(a);
			return !DisappearTest();
		}

		void Render() override
		{
			EnemyShot::Render();
			graphics->SetColor(kxxwin->defaultFontColor);
			graphics->SetFont(kxxwin->debugFont);
			graphics->DrawString(pos.x, pos.y + (hsz.y * scl.y), name);
		}
	};
}

#endif
