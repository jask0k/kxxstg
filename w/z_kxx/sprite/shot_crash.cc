
// #include "../config.h"
#include <z_kxx/sprite/shot_crash.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	//bool CalcRingVertices(Vector2 _buf[], size_t _size, float _innerRadius, float _outerRadous)
	//{
	//	if(_size < 8)
	//	{
	//		E_ASSERT(0);
	//		return false;
	//	}

	//	size_t sectionCount = (_size - 2) / 2;
	//	float da = PI * 2 / sectionCount;
	//	size_t sz2 = _size - 2;
	//	float a = 0;
	//	for(size_t i = 0; i < sz2;i+=2, a+=da)
	//	{
	//		float cos_a = cos(a);
	//		float sin_a = sin(a);
	//		_buf[i+0].x = cos_a * _outerRadous;
	//		_buf[i+0].y = sin_a * _outerRadous;
	//		_buf[i+1].x = cos_a * _innerRadius;
	//		_buf[i+1].y = sin_a * _innerRadius;
	//	}
	//	_buf[_size-1] = _buf[1];
	//	_buf[_size-2] = _buf[0];
	//	return true;
	//}
	static const int SECTION_COUNT = 16;
	ShotCrash::ShotCrash(TexRef & _tex)
	{
		tex = _tex;
		size = SECTION_COUNT * 2 + 2;
		v = enew VCT[size];
		timer0 = 120;
		timer1 = 20;
		alpha = 100;
		alpha_delta = -0.99f / timer0;
		w = 0.99f;
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;
		float r = sqrt(hsz.y *hsz.y + hsz.x*hsz.x);
		hsz.x = r;
		hsz.y = r;
		// memset(v, 0, sizeof(VCT) * size);
		float da = PI * 2 / SECTION_COUNT;
		size_t sz2 = size - 2;
		float a = 0;
		for(size_t i = 0; i < sz2; i+=2, a+=da)
		{
			float cos_a = cos(a);
			float sin_a = sin(a);
			v[i+0].v.z = 0;
			v[i+0].t.x = 0.5f + 0.5f * cos(a) * r / hsz.x;
			v[i+0].t.y = 0.5f + 0.5f * sin(a) * r / hsz.x;
			v[i+1].v.z = 0;
			v[i+1].t.x = 0.5f;
			v[i+1].t.y = 0.5f;
		}
		ChangeW();
	}

	ShotCrash::~ShotCrash()
	{
		delete[] v;
	}

	void ShotCrash::ChangeW()
	{
		E_ASSERT(size >= 8);
		float outer_r = hsz.x;
		float inner_r = hsz.x * (1.0f - w);
		RGBA rgba0 = {alpha*0.5f, alpha*0.5f, alpha*0.5f, alpha*0.5f};
		RGBA rgba1 = {alpha*0.7f, alpha*0.7f, alpha*0.7f, alpha*0.7f};
		InternalColor c0 = graphics->RGBAtoInternalColor(rgba0);
		InternalColor c1 = graphics->RGBAtoInternalColor(rgba1);
		float da = PI * 2 / SECTION_COUNT;
		size_t sz2 = size - 2;
		float a = 0;
		for(size_t i = 0; i < sz2; i+=2, a+=da)
		{
			float cos_a = cos(a);
			float sin_a = sin(a);
			v[i+0].v.x = cos_a * outer_r;
			v[i+0].v.y = sin_a * outer_r;
			v[i+0].c = c0;
			v[i+1].v.x = cos_a * inner_r;
			v[i+1].v.y = sin_a * inner_r;
			v[i+1].c = c1;
		}
		v[size-1] = v[1];
		v[size-2] = v[0];	
	}

	bool ShotCrash::Step()
	{
		if(--timer0 == 0)
		{
			return false;
		}

		if(--timer1 == 0)
		{
			timer1 = 20;
			w= 0.68f * w;
			ChangeW();
		}

		clr.a+= alpha_delta;
		scl.x+= 0.015f;
		scl.y+= 0.015f;

		return true;
	}

	void ShotCrash::Render()
	{
		graphics->SetBlendMode(BM_NORMAL);
		graphics->SetColor(1, 1, 1, 1);
		graphics->BindTex(tex);
		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);
		graphics->RotateMatrix(ang, 0, 0, -1);

		graphics->SetVertexSource(v, sizeof(VCT), VCT::FORMAT, size);
		graphics->DrawPrimitive(E_TRIANGLESTRIP, 0, size);

		graphics->PopMatrix();
		graphics->SetBlendMode(BM_NORMAL);
	}
}
