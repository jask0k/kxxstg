
#pragma once
#include <nbug/core/def.h>
#include <nbug/core/callback.h>

namespace e
{
	// FPS calcuator
	// * calc FPS
	// * calc load
	// * auto frame skip
	class FpsCalculator
	{
	public:
		FpsCalculator();
		~FpsCalculator();
		enum State
		{
			stateIdle,   // do nothing
			stateRender, // reander an logic 
			stateSkip,   // logic only
		};
		State Step();
		double GetRenderFPS() const
		{ return renderFPS; }
		double GetSkipedFPS() const
		{ return skipedFPS; }
		int GetThisCycleFrame() const
		{ return thisCycleRenderedFrames; }
		double GetLoad() const
		{ return load; }
	private:
		uint   continuousSkipCount;
		double renderFrameCount;
		double busyTimeSpan;
		double skipedFrameCount;
		double renderFPS;
		double skipedFPS;
		double renderFrameCycleT0;
		State  lastState;
		int    thisCycleRenderedFrames;
		double load; 
		double prevT0;
	};
}

