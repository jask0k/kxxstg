
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class SimpleSprite : public Sprite
	{
	public:
		SimpleSprite(const stringa & _name);
		~SimpleSprite();
		void Render() override;
	};
}

