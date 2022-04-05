
//#include "private.h"
#include <nbug/tl/map.h>
#include <nbug/core/debug.h>
#include <nbug/core/ref.h>

namespace e
{
	typedef e::Map<void *, int *> RefCounterPointerMap;
	static RefCounterPointerMap g_ref_counter_pointer_map;
	int * g_ref_get_counter(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		RefCounterPointerMap::iterator it = g_ref_counter_pointer_map.find(_p);
		if(it == g_ref_counter_pointer_map.end())
		{
			int * pc = enew int;
			*pc = 0;
			g_ref_counter_pointer_map[_p] = pc;
			return pc;
		}
		else
		{
			return it->second;
		}
	}

	void g_ref_del_counter(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		g_ref_counter_pointer_map.erase(_p);
	}
}

