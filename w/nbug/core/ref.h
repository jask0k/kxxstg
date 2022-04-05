
#pragma once

#include <nbug/core/debug.h>
#include <nbug/core/obj.h>

namespace e
{

	// Ref's "addr ==> counter" map
	int * g_ref_get_counter(void * _p);
	void  g_ref_del_counter(void * _p);

	// auto Ref counting smart pointer, similar to share_ptr, but not equal.
	// Ref has map table, ensure same addr has same Ref count. this is regular usage:
	// 
	// char * p = enew char;
	// Ref<char> p0 = p;  // Ref count is 1
	// Ref<char> p1 = p;  // correct, Ref count is 2
	// 
	template <typename T> class ExtRef
	{
		T * p;
		int * c;
	public:
		ExtRef(T * _p = 0)
		{
			p = _p;
			if(_p)
			{
				c = g_ref_get_counter(_p);
				(*c)++;
			}
		}

		ExtRef(const ExtRef<T> & _r)
			: p(_r.p)
			, c(_r.c)
		{
			if(p)
			{
				(*c)++;
			}
		}

		void Detach()
		{
			if(p)
			{
				if(--(*c) == 0)
				{
					g_ref_del_counter(p);
					delete p;
					delete c;
				}
				p = 0;
			}
		}

		~ExtRef()
		{
			Detach();
		}

		operator bool() const
		{ return p != 0; }
		T & operator *()
		{ 
			E_ASSERT(p);
			return *p; 
		}
		const T & operator *() const
		{ 
			E_ASSERT(p);
			return *p; 
		}
		T * operator->()
		{ 
			E_ASSERT(p);
			return p; 
		}
		const T * operator->() const
		{ 
			E_ASSERT(p);
			return p; 
		}
		T * ptr()
		{ return p; }
		const T * ptr()const
		{ return p; }

		const ExtRef & operator=(const ExtRef & _r)
		{
			if(this != & _r)
			{
				if(_r.p)
				{
					(*_r.c)++;
				}				
				Detach();
				p = _r.p;
				c = _r.c;
			}
			return * this;
		}

		bool operator==(const ExtRef & _r) const
		{ return p == _r.p; }
		bool operator==(const T * _p) const
		{ return p == _p; }
		//int RefCount() const
		//{ return *c; }

	};

	// 
	template <typename T> class Ref
	{
		T * p;
	public:
		Ref(T * _p = 0)
		{
			p = _p;
			if(_p)
			{
				_p->AddRef();
			}
		}

		Ref(const Ref<T> & _r)
			: p(_r.p)
		{
			if(p)
			{
				p->AddRef();
			}
		}

		void Detach()
		{
			if(p)
			{
				p->Release();
				p = 0;
			}
		}

		~Ref()
		{
			Detach();
		}

		operator bool() const
		{ return p != 0; }
		T & operator *()
		{ 
			E_ASSERT(p);
			return *p; 
		}
		const T & operator *() const
		{ 
			E_ASSERT(p);
			return *p; 
		}
		T * operator->()
		{ 
			E_ASSERT(p);
			return p; 
		}
		const T * operator->() const
		{ 
			E_ASSERT(p);
			return p; 
		}
		operator T*() 
		{ return p; }
		operator const T *() const
		{ return p; }
		T * ptr()
		{ return p; }
		const T * ptr()const
		{ return p; }

		const Ref & operator=(const Ref & _r)
		{
			if(this != & _r)
			{
				if(_r.p)
				{
					_r.p->AddRef();
				}				
				Detach();
				p = _r.p;
			}
			return * this;
		}

		bool operator==(const Ref & _r) const
		{ return p == _r.p; }
		bool operator==(const T * _p) const
		{ return p == _p; }
		//int RefCount() const
		//{ return c; }

	};
}

