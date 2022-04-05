
#pragma once

#include <nbug/tl/list.h>

namespace e
{
	static const int GRIDW = 43;
	static const int GRIDH = 50;
	static const int GRIDSIZE = GRIDW * GRIDH;
	//class Fbo;
	class Win;
	class LensEffect
	{
	protected:
		friend class KxxWin;
		uint32 timer;
		Graphics * g;
		float w, h;
		Vector2 pos;
		FboRef fbo;
		virtual void Init(Graphics * _g, float _x, float _y, float _w, float _h);

		struct MASS
		{
			Vector2 tc;
			Vector2 pos;
			Vector2 pos0;
			Vector2 v;
			Vector2 f;
			bool fixed;
		};
		
		union
		{
			MASS grid1[GRIDSIZE];
			MASS grid2[GRIDW][GRIDH];
		};

		struct Spring
		{
			int a;
			int b;
			float k;
			float l;
		};
		List<Spring> spring1;

	public:
		LensEffect();
		~LensEffect();
		void Render();
		void Step(float _x, float _y);
		//void UpdateViewport();
	};
}

