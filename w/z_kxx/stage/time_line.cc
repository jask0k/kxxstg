
// #include "../config.h"
#include <z_kxx/stage/time_line.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	Timeline::Timeline()
	{
		eventList = 0;
		pendingEvent = 0; 
		nestTimeLine = 0;
		timeTick = 0;
		pendingNestTimeLine = 0;
		pause = false;
	}

	Timeline::~Timeline()
	{
		TimelineEvent * p;
		TimelineEvent * p1 = 0;
		p = eventList;
		while(p)
		{
			p1 = p;
			p = p->next;
			delete p1;
		}
	}

	void Timeline::Start()
	{
		pendingEvent = eventList;
	//	kxxwin->ClearEnemyShots(true, 0);
	}

	bool Timeline::Step()
	{
		if(pause)
		{
			return true;
		}

		if(pendingNestTimeLine)
		{
			nestTimeLine = pendingNestTimeLine;
			nestTimeLine->Start();
			pendingNestTimeLine = 0;
		}

		if(nestTimeLine)
		{
			nestTimeLine->Step();
			if(nestTimeLine->IsStoped())
			{
				nestTimeLine = 0;
			}
			return true;
		}

		if(IsStoped())
		{
			return false;
		}

		//E_TRACE_LINE(L"timeTick = " + string((int)timeTick));
		if(timeTick == pendingEvent->time)
		{
			for(uint i=0; i<pendingEvent->handlers.size(); i++)
			{
				TIMELINE_HANDLER & handler = pendingEvent->handlers[i];
				if(handler.func)
				{
					handler.func(this, handler.param0, handler.param1);
				}
			}

			// next pending event
			pendingEvent = pendingEvent->next;
		}
		timeTick++;
		return IsRunning();
	}

	void Timeline::RegisterTimeEvent(uint _time, TIMELINE_HANDLER_FUNC _handler, int _param0, int _param1)
	{
		E_ASSERT(!IsRunning());
		TimelineEvent * p;
		TimelineEvent * prev = 0;
		p = eventList;
		while(p && p->time < _time)
		{
			prev = p;
			p = p->next;
		}
		
		if(p != 0 && p->time == _time)
		{
			TIMELINE_HANDLER h;
			h.func = _handler;
			h.param0 = _param0;
			h.param1 = _param1;
			p->handlers.push_back(h);
			return;
		}

		E_ASSERT(p==0 || p->time > _time);

		TimelineEvent * current = enew TimelineEvent();
		current->next = p;
		TIMELINE_HANDLER h;
		h.func = _handler;
		h.param0 = _param0;
		h.param1 = _param1;
		current->handlers.push_back(h);
		current->time = _time;
		
		if(prev)
		{
			E_ASSERT(prev->next == p);
			prev->next = current;
		}
		else
		{
//			E_ASSERT(eventList == 0);
			eventList = current;
		}
	}

	void Timeline::RegisterTimeEvents(uint _t0, uint _dt, uint _repeat, TIMELINE_HANDLER_FUNC _handler, int _param0, int _param1)
	{
		for(uint i=0; i<_repeat; i++)
		{
			RegisterTimeEvent(_t0, _handler, _param0, _param1);
			_t0+= _dt;
		}
	}


	void Timeline::SwitchToNextEvent()
	{
		if(pendingEvent)
		{
			timeTick = pendingEvent->time - 1;
		}
	}

	void Timeline::RunNestTimeLine(Timeline * _nest)
	{
		E_ASSERT(_nest);
		E_ASSERT(pendingNestTimeLine == 0); 
		E_ASSERT(nestTimeLine == 0);
		pendingNestTimeLine = _nest;
	}


}

