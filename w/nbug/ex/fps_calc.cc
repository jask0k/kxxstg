
// #include "private.h"
#include <nbug/ex/fps_calc.h>
#include <nbug/core/debug.h>
#include <nbug/core/time.h>

namespace e
{
	FpsCalculator::FpsCalculator()
	{
		continuousSkipCount = 0;
		renderFrameCount = 0;
		skipedFrameCount = 0;	
		renderFPS = 0;
		skipedFPS = 0;
		thisCycleRenderedFrames = 0;
		renderFrameCycleT0 = Time::GetTicks();
		lastState   = stateIdle;
		load = 1.0;
		busyTimeSpan = 0;
		prevT0 = Time::GetTicks();
	}


	FpsCalculator::~FpsCalculator()
	{
	}

	FpsCalculator::State FpsCalculator::Step()
	{
		// E_PROFILE_INCLUDE

		double t = Time::GetTicks();
		if(lastState == stateSkip || lastState == stateRender)
		{
			busyTimeSpan+= t - prevT0;
		}

		double renderFrameSpan = t - renderFrameCycleT0;

		// calc load and FPS one time per second
		double totalFrameCount = renderFrameCount + skipedFrameCount;
		if(totalFrameCount >= 60)
		{
			renderFrameCycleT0 = t;
			renderFPS = renderFrameCount / renderFrameSpan;
			skipedFPS = skipedFrameCount / renderFrameSpan;
			if(renderFrameCount > 0)
			{
				double newLoad = busyTimeSpan * 60 / renderFrameSpan / renderFrameCount;
				load = newLoad;
			}

			renderFrameCount = 0;
			skipedFrameCount = 0;
			renderFrameSpan = 0;
			busyTimeSpan = 0;
			//E_TRACE_LINE("load=" + string(load));
		}
		else if(renderFrameSpan < 0)
		{
			renderFrameCycleT0 = t;
			renderFrameSpan = 0;
		}

		prevT0 = t;
		int n = 60;
		double nFrameShouldBe = renderFrameSpan * n;

		// sleep a while when we draw to fast, 0.5 is experimental value.
		// BUG: Load is wrong in the case of vsync on
		if(nFrameShouldBe < renderFrameCount + (1.1-load)*0.5)
		{
			lastState = stateIdle;
			return stateIdle;
		}

		thisCycleRenderedFrames++;
		if(thisCycleRenderedFrames >= 60)
		{
			thisCycleRenderedFrames = 0;
		}

		State ret;
		
		static const double threshold_a = 2; 
		static const int max_continous_skip = 1;

		if(continuousSkipCount >= max_continous_skip // retrict continuous skip
			|| nFrameShouldBe < totalFrameCount + threshold_a)
		{
			renderFrameCount++;
			continuousSkipCount = 0;
			ret = stateRender;
		}
		else
		{
			continuousSkipCount++;
			skipedFrameCount++;
			ret = stateSkip;
		}
		lastState = ret;
		return ret;
	}

}
