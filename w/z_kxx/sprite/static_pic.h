
#pragma once

#include <z_kxx/sprite/sprite.h>
#include <nbug/gl/rect.h>
namespace e
{
	class StaticImage : public Sprite
	{
	public:
		Rect bound;
		StaticImage(const stringa & _name, float _x0, float _y0, float _x1, float _y1);
		~StaticImage();
		void Render() override;
	};
}

