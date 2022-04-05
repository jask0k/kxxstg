
#pragma once

#include <nbug/core/obj.h>
#include <nbug/core/debug.h>
#include <nbug/core/def.h>

namespace e
{
	class Callback 
	{
		typedef int (*GLOBAL_FUNC)(void *);
		typedef int (Object::*OBJECT_FUNC)(void *);
		Object * o;
		union
		{
			GLOBAL_FUNC gf;
			OBJECT_FUNC mf;
		};
		Callback(void*, void*); // fake c_tor
	public:
		Callback()
		{ o = 0; mf = 0; E_ASSERT(sizeof(mf) >= sizeof(gf)); }

		Callback(GLOBAL_FUNC _gf)
		{ o = 0; mf = 0; gf = _gf; }

		template <typename T> Callback(T * _obj, int (T::*_mf)(void *) )
		{
			o = _obj;
			E_ASSERT(sizeof(mf) == sizeof(_mf));
			mf = (OBJECT_FUNC) _mf;
		}

		int operator()(void * _a)
		{ return o ? (o->*mf)(_a) : (gf ? gf(_a) : 0); }

		bool operator==(const Callback & _r) const
		{ return  o == _r.o && mf == _r.mf; }

		const Callback & operator=(const Callback & _r)
		{ o = _r.o; mf = _r.mf;  return *this; }

		operator bool() const
		{ return o != 0; }
	};
}

