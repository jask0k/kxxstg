
// #include "../config.h"
#include <z_kxx/sprite/float_text.h>

namespace e
{
	// E_DEFINE_MEMORY_POOL(FloatText);

	FloatText::FloatText()
	{
		font = 0;
		life = 30;
		speed = 2;
	}

	FloatText::~FloatText()
	{
	}

	void FloatText::Render()
	{
		if(!font)
		{
			return;
		}
		//clr.a = 190 + life * 2;
		graphics->SetColor(clr);
		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->SetFont(font);
		graphics->DrawString(pos.x, pos.y, text);
		// graphics->SetTexMode(TextureMode::replace);
	}
	

	bool FloatText::Step()
	{
		//if(_mf)
		//{
		//	return true;
		//}

		if(life)
		{
			life--;
			pos.y-= speed;
			return true;
		}
		else
		{
			return false;
		}
	}
}
