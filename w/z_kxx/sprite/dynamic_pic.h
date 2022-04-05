
#pragma once

#include <z_kxx/sprite/sprite.h>
#include <nbug/gl/rect.h>
namespace e
{
	class DynamicImage : public Sprite
	{
		int state;
		uint32 timer[3];
		uint32 timerMax[3];
		float GetAlpha()
		{
			switch(state)
			{
			case 0:
				return 1.0f - float(timer[0]) / timerMax[0];
			case 1:
				return 1;
			case 2:
				return float(timer[2]) / timerMax[2];
			}
			return 0;
		}
		bool autoMode;
	public:
		Rect bound;
		enum Method
		{
			Alpha,
			FlipH,
			FlipV,
		}method;
		DynamicImage(const stringa & _name, float _x0, float _y0, float _x1, float _y1);
		~DynamicImage();
		void SetTimer(uint32 _in, uint32 _keep, uint32 _out, bool _auto);
		void FadeOut();
		void Render() override;
		bool Step() override;
	};
}

