
// #include "../config.h"
#include <z_kxx/sprite/ring.h>
//#include "OldGraphics.h"

namespace e
{
	bool CalcRingVertices(Vector2 _buf[], size_t _size, float _innerRadius, float _outerRadous)
	{
		if(_size < 8)
		{
			E_ASSERT(0);
			return false;
		}

		size_t sectionCount = (_size - 2) / 2;
		float da = PI * 2 / sectionCount;
		size_t sz2 = _size - 2;
		float a = 0;
		for(size_t i = 0; i < sz2;i+=2, a+=da)
		{
			float cos_a = cos(a);
			float sin_a = sin(a);
			_buf[i+0].x = cos_a * _outerRadous;
			_buf[i+0].y = sin_a * _outerRadous;
			_buf[i+1].x = cos_a * _innerRadius;
			_buf[i+1].y = sin_a * _innerRadius;
		}
		_buf[_size-1] = _buf[1];
		_buf[_size-2] = _buf[0];
		return true;
	}

	Ring::Ring(TexRef _tex, float _innerRadius, float _outerRadous, float _angleDelta, uint _sectionCount)
	{
		tex = _tex;
		size = _sectionCount * 2 + 2;
		vertices = enew Vector2[size];
		angleDelta = _angleDelta;
		SetRadius(_innerRadius, _outerRadous);
	}

	Ring::~Ring()
	{
		delete[] vertices;
	}

	void Ring::SetRadius(float _innerRadius, float _outerRadous)
	{
		innerRadius = _innerRadius;
		outerRadous = _outerRadous;
		CalcRingVertices(vertices, size, innerRadius, outerRadous);
	}
	
	void Ring::SetScale(float _scale)
	{
		scl.x = scl.y =_scale;
	}

	void Ring::Render()
	{
		//Sprite::Render();
		graphics->SetBlendMode(blm);
		graphics->SetColor(clr);
		graphics->BindTex(tex);
		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);
		graphics->RotateMatrix(ang, 0, 0, -1);
		graphics->ScaleMatrix(scl.x, scl.y, 1);
		graphics->DrawQuadStripX(vertices, size);
		graphics->PopMatrix();
		graphics->SetBlendMode(BM_NORMAL);
	}


	bool Ring::Step()
	{
		ang+= angleDelta;
		return true;
	}

}
