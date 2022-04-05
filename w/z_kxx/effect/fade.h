
#pragma once

#include <nbug/gl/graphics.h>

namespace e
{
	//
	// general step
	// * capture last screen before translate (A)
	// * capture current screen continuously(B)
	// * mixing (A) and (B) with proper method =>(A+B)
	class Fade : public Object
	{
	protected:
		friend class KxxWin;
		bool overlay;
		uint32 duration;
		uint32 counter;
		Graphics * g;
//		int viewport_w, viewport_h;
		float w, h;
		Fade(bool _ca, bool _cb);
		FboRef fboA;
		FboRef fboB;
		virtual void Init(Graphics *_g, float _w, float _h, uint32 _duration);
		bool Step();
		const bool captureA;
		const bool captureB;
		float GetFrac() const
		{ return (float)counter / (float)duration; }
	public:
		~Fade();

		// output
		virtual void Render() = 0;
	};

}

