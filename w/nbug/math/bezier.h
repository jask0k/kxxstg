#ifndef NB_BEZIER_H
#define NB_BEZIER_H

namespace e
{
	static inline void cubic_bezier(float & _x, float & _y, float t,
		float _x0, float _y0, float _x1, float _y1, 
		float _x2, float _y2, float _x3, float _y3)
	{
		float t2 = t * t;
		float t3 = t2 * t;
		float nt = 1 - t;
		float nt2 = nt * nt;
		float nt3 = nt2 * nt;
		_x = nt3*_x0 + 3*nt2*t*_x1 + 3*nt*t2*_x2 + t3*_x3;
		_y = nt3*_y0 + 3*nt2*t*_y1 + 3*nt*t2*_y2 + t3*_y3;
	}
}

#endif