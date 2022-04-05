
//#include "private.h"

#ifdef E_CFG_DYNAMIC_CREATE
#	include <nbug/tl/map.h>
#endif

#include <nbug/core/obj.h>
#include <nbug/core/str.h>
#include <nbug/core/debug.h>

namespace e
{
#ifdef E_CFG_DYNAMIC_CREATE
	typedef e::Map<stringa, Rtti*> DynamicClassMap;
	DynamicClassMap * g_dynamic_class_map = 0;

	static void DeleteClassMap()
	{
		delete g_dynamic_class_map;
	}

	Rtti * Rtti::Find(const stringa & _className)
	{
		if(g_dynamic_class_map)
		{
			DynamicClassMap::iterator it = g_dynamic_class_map->find(_className);
			if(it != g_dynamic_class_map->end())
			{
				return it->second;
			}
		}
		E_THROW(string("[nb] FindClass(\"") + _className.c_str() + "\"): not found. ");
	}

	void Rtti::Register(const stringa & _className, Rtti * _class)
	{
		E_ASSERT(_className.length() < 250); // long class name?
		E_ASSERT(_className.find(':') != -1); // class name must include namespace. E_CLASS(ns::, A);
		if(g_dynamic_class_map == 0)
		{
			g_dynamic_class_map = enew DynamicClassMap();
			atexit(&DeleteClassMap);
		}
#ifdef NB_DEBUG
		else
		{
			DynamicClassMap::iterator it = g_dynamic_class_map->find(_className);
			if(it != g_dynamic_class_map->end())
			{
				E_TRACE_LINE(string("[nb] RegisterClass(\"") + _className.c_str() + "\", ...): already registered.");
			}
		}
#endif
		(*g_dynamic_class_map)[_className] = _class;
		E_TRACE_LINE(string("[nb] RegisterClass(\"") + _className.c_str() + "\", ...): OK.");
	}

	Rtti * Object::GetRtti() const
	{
		return 0; // Object itself has not rtti info
	}
#endif

	Object::~Object()
	{}

}
