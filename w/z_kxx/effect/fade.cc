
// #include "../config.h"
#include "fade.h"

namespace e
{
	Fade::Fade(bool _ca, bool _cb)
		: g(0)
		, w(0)
		, h(0)
		, captureA(_ca)
		, captureB(_cb)
	{
		fboA = 0;
		fboB = 0;
		duration = 1;
		counter = 0;
		overlay = !_cb;
	}
	
	Fade::~Fade()
	{
		//if(fboA)
		//{
		//	fboA->Release();
		//}
		//if(fboB)
		//{
		//	fboB->Release();
		//}
	}
	
	void Fade::Init(Graphics *_g, float _w, float _h, uint32 _duration)
	{
		E_ASSERT(g == 0);
		g = _g;
		w = _w;
		h = _h;

		if(_duration == 0)
		{
			_duration = 1;
		}
		duration = _duration;
		counter = 0;
		if(captureA)
		{
			E_ASSERT(_g->GetCapabilities().frameBufferObject);
			fboA = _g->CreateFbo((int)w, (int)h);
		}
		if(captureB)
		{
			E_ASSERT(_g->GetCapabilities().frameBufferObject);
			fboB = _g->CreateFbo((int)w, (int)h);
		}
	}

	bool Fade::Step()
	{
		if(counter < duration)
		{
			counter++;
			return true ;
		}
		else
		{
			return false;
		}
	}
}
