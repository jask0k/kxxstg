
#ifndef NB_TIME_H
#define NB_TIME_H

#include <time.h>
#include <nbug/core/str.h>
#include <nbug/core/def.h>

namespace e
{
	class Time
	{
	public:
		time_t v;
		static Time Current();
		string GetString(const Char * _format, bool _utc)const;
		string GetString() const;
		static double GetTicks(); // s
	};
}

#endif
