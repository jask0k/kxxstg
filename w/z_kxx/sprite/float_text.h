
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class FloatText : public Sprite
	{
	public:
		string text;
		FontRef font;
		RGBA clr;
		uint life;
		float speed;
		FloatText();
		~FloatText();
		void Add(uint _n);
		void Render() override;
		bool Step() override;
	};
}

