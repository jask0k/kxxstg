#pragma once

#include <nbug/gl/graphics.h>

namespace e
{
	class DigitalRoller
	{
	//	int shift[10];
		float rot[9];
		float x_offset[9];
		float y_offset;
		int num[9];
		float num_delta_angle;
		float num_fix_angle[20];
		float num_current_angle[9];
		float num_target_angle[9];
		float r;
		static const int YC = 16;
		VCT v[9][YC*2];
		TexRef strip_tex[5];
		TexRef panel_tex;
		int display_num;
		int target_num;
	public:
		DigitalRoller();
		void Set(int _v);
		void Step();
		void Render(float _x, float _y);
	};
}

