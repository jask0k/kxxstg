
#pragma once
#include <nbug/tl/array.h>
#include <nbug/core/def.h>
namespace e
{

	class Timeline;

	typedef void (*TIMELINE_HANDLER_FUNC)(Timeline *, int, int);

	struct TIMELINE_HANDLER
	{
		int param0;
		int param1;
		TIMELINE_HANDLER_FUNC func;
	};

	struct TimelineEvent
	{
		uint32 time;
		Array<TIMELINE_HANDLER> handlers;
		TimelineEvent * next;
	};

	//class KxxTimerObject
	//{
	//public:
	//	virtual void Start() = 0;
	//	virtual bool Step() = 0;
	//	virtual ~KxxTimerObject(){};
	//};

	class Timeline
	{
		TimelineEvent * eventList;
		TimelineEvent * pendingEvent;
		Timeline * nestTimeLine;
		bool pause;
		uint32 timeTick;
		Timeline * pendingNestTimeLine; 
		void SwitchToNextEvent();
	public:

		Timeline();
		virtual ~Timeline();
		virtual void Start();
		virtual bool Step();
		bool IsStoped() const
		{ return pendingEvent == 0; }

		bool IsRunning() const
		{ return pendingEvent != 0; }

		uint32 GetTimeTick() const
		{ return timeTick; }

		void RunNestTimeLine(Timeline * _nest);
		void RegisterTimeEvent(uint _time, TIMELINE_HANDLER_FUNC _handler, int _param0, int _param1 = 0);
		void RegisterTimeEvents(uint _t0, uint _dt, uint _repeat, TIMELINE_HANDLER_FUNC _handler, int _param0, int _param1 = 0);
	};
}
