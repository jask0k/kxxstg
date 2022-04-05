
#pragma once

#include <nbug/core/debug.h>
#include <nbug/core/nd_memory.h>

namespace e
{
	// 
	// 
	template <typename T> class SimpleList
	{
	public:
		T * head;
		T * tail;

		SimpleList() 
			: head(0)
			, tail(0)
		{}

		~SimpleList()
		{ 
			delete_all(); 
		}

		void push_back(T * _p)
		{
			_p->prev = tail;
			_p->next = 0;
			if(tail)
			{
				tail->next = _p;
				tail = _p;
			}
			else
			{
				tail = _p;
				head = _p;
			}
		}

		T * remove(T * _p)
		{
			return remove_then_next(_p);
		}

		T * remove_then_next(T * & _p)
		{
			T * ret = _p;
			if(_p->prev)
			{
				_p->prev->next = _p->next;
			}
			else
			{
				head = reinterpret_cast<T*>(_p->next);
				if(head == 0)
				{
					tail = 0;
				}
			}

			if(_p->next)
			{
				_p->next->prev = _p->prev;
			}
			else
			{
				tail = reinterpret_cast<T*>(_p->prev);
				if(tail == 0)
				{
					head = 0;
				}
			}
			_p = reinterpret_cast<T*>(_p->next);
			return ret;
		}

		bool empty()
		{ return head == 0; }

		void delete_all()
		{
			while(head)
			{
				T * p = head;
				head = reinterpret_cast<T*>(head->next);
				delete p;
			}
			head = 0;
			tail = 0;
		}

		void reomve_all()
		{
			head = 0;
			tail = 0;
		}

		void join(SimpleList & _r)
		{
			T * p = _r.head;
			while(p)
			{
				T * p1 = p;
				p = p->next;
				push_back(_r.remove(p1));
			}
		}

		int size() const
		{
			int ret = 0;
			T * p = head;
			while(p)
			{
				ret++;
				p = reinterpret_cast<T*>(p->next);
			}
			return ret;
		}

		static void move_next(T * & _p)
		{
			_p = reinterpret_cast<T*>(_p->next);
		}
	};
}

