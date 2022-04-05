
//#include "private.h"
#ifdef NB_WINDOWS
#	include <windows.h>
#endif

#ifdef NB_LINUX
#	include <unistd.h>
#	include <sys/time.h>
#endif

#include <nbug/core/time.h>
#include <nbug/core/str.h>
#include <nbug/core/debug.h>
#include <nbug/core/thread.h>

namespace e
{
	Time Time::Current()
	{
		Time ret;
		time(&ret.v);
		return ret;
	}
	/*
	uint64 Time::GetTicks64()
	{
#ifdef NB_WINDOWS
		if(!g_freq_ready)
		{
			get_freq();
		}
		uint64 ret = 0, freq = 0;
		if(QueryPerformanceCounter((LARGE_INTEGER*)&ret) &&
			QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
		{
			E_ASSERT(ret < 18446744073709LL);
			return (ret * 1000000) / freq;
		}
		else
		{
			return uint64(::GetTickCount()) * 1000;
		}
#endif

#ifdef NB_LINUX
		timeval tm;
		gettimeofday(&tm, NULL);
		return tm.tv_sec * 1000000L + tm.tv_usec;
#endif
	}
*/
	double Time::GetTicks()
	{
#ifdef NB_WINDOWS
		static double _freq_double;
		static bool   _freq_ready = false;
		if(!_freq_ready)
		{
			uint64 freq;
			if(QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
			{
				_freq_double = (double) freq;
				_freq_ready = true;
			}
			else
			{
				_freq_double = 1;
			}
		}

		uint64 ret;
		if(QueryPerformanceCounter((LARGE_INTEGER*)&ret))
		{
			return double(ret) / _freq_double;
		}
		else
		{
			return double(::GetTickCount()) / 1000.0;
		}
#endif

#ifdef NB_LINUX
		timeval tm;
		int rc = gettimeofday(&tm, NULL);
		if(rc == 0)
		{
			return double(tm.tv_sec) + double(tm.tv_usec) / 1000000.0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
#endif
	}

	/*
	TimeSpan Time::operator-(const Time & _r) const
	{
		TimeSpan ret;
		ret.v = difftime(v, _r.v);
		return ret;
	}
	*/

	string Time::GetString() const
	{
		return GetString(L"%Y-%m-%d", false);
	}

	string Time::GetString(const Char * _format, bool _utc) const
	{
		E_ASSERT(_format != 0);
#ifdef _MSC_VER
		struct tm tm1;
		struct tm * timeinfo = &tm1;
		if(_utc)
		{
			errno_t err = _gmtime64_s(timeinfo, &v);
		}
		else
		{
			errno_t err = _localtime64_s(timeinfo, &v);
		}
#else
		struct tm * timeinfo;
		timeinfo = _utc ? gmtime(&v) : localtime(&v);
#endif

#ifdef NB_LINUX
		stringa ret;
		ret.reserve(1024);
		stringa format1(_format);
		strftime(ret.c_str(), 1024, format1.c_str(), timeinfo);
#else
		string ret;
		ret.reserve(1024);
#	ifdef UNICODE
		wcsftime(ret.c_str(), 1024, _format, timeinfo);
#	else
		strftime(ret.c_str(), 1024, _format, timeinfo);
#	endif
#endif
		return ret;
	}

}
