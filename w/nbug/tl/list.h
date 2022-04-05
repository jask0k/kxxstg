
#pragma once

#include <new>
#include <nbug/core/debug.h>
//#include <nbug/core/nd_memory.h>
// conflict with <new> header file
//inline void * operator new(size_t, void *_p)
//{ return (_p); }
//
//inline void  operator delete(void *, void *)
//{ }
//
//inline void * operator new[](size_t, void * _p)
//{ return (_p); }
//
//inline void operator delete[](void *, void *)
//{ }
namespace e
{
	template <typename T> class List
	{
		struct Node
		{
			Node * next;
			Node * prev;
			T data;
		};
		Node * head;
		Node * tail;
	public:
		class iterator
		{
			friend class List;
			Node * p;
#ifdef NB_DEBUG
			List * owner;
			iterator(Node * _p, List * _owner) 
				: p (_p), owner(_owner)
			{}
#else
			iterator(Node * _p) 
				: p (_p)
			{}
#endif
		public:
			bool operator==(const iterator & _r) const
			{ 
				E_ASSERT(owner == _r.owner);
				return p == _r.p; 
			}
			bool operator!=(const iterator & _r) const
			{ 
				E_ASSERT(owner == _r.owner);
				return p != _r.p; 
			}
			iterator & operator++()
			{
				E_ASSERT(p);
				p = p->next;
				return *this;
			}
			T & operator*()
			{
				E_ASSERT(p);
				return p->data;
			}
			T * operator->()
			{
				E_ASSERT(p);
				return &p->data;
			}
		};

		iterator begin()
		{
#ifdef NB_DEBUG
			return iterator(head, this);
#else
			return iterator(head);
#endif
		}

		iterator end()
		{
#ifdef NB_DEBUG
			return iterator(0, this);
#else
			return iterator(0);
#endif
		}

		List() 
			: head(0)
			, tail(0)
		{}

		~List()
		{ 
			clear();
		}

		void push_back(const T & _data)
		{
			Node * p = (Node*) nd_malloc(sizeof(Node));
			new(&p->data) T(_data);
			p->prev = tail;
			p->next = 0;
			if(tail)
			{
				tail->next = p;
				tail = p;
			}
			else
			{
				tail = p;
				head = p;
			}
		}

		bool empty()
		{ return head == 0; }

		int size() const
		{
			int ret = 0;
			Node * p = head;
			while(p)
			{
				ret++;
				p = p->next;
			}
			return ret;
		}

		void erase(const T & _v)
		{
			iterator it = find(_v);
			if(it != end())
			{
				erase(it);
			}
		}

		iterator find(const T & _v)
		{
			Node * p;
			p = head;
			while(p)
			{
				if(p->data == _v)
				{
					break;
				}
				p = p->next;
			}

#ifdef NB_DEBUG
			return iterator(p, this);
#else
			return iterator(p);
#endif
		}

		iterator erase(const iterator & it)
		{
			E_ASSERT(it.owner == this);
			Node * p = it.p;
			Node * next = p->next;
			if(p->prev)
			{
				p->prev->next = p->next;
			}
			else
			{
				head = p->next;
				if(head == 0)
				{
					tail = 0;
				}
			}

			if(p->next)
			{
				p->next->prev = p->prev;
			}
			else
			{
				tail = p->prev;
				if(tail == 0)
				{
					head = 0;
				}
			}
			p->data.~T();
			nd_free(p);
#ifdef NB_DEBUG
			return iterator(next, this);
#else
			return iterator(next);
#endif
		}

		iterator insert(const iterator & it, const T & _data)
		{
			E_ASSERT(it.owner == this);
			Node * p = (Node*) nd_malloc(sizeof(Node));
			new(&p->data) T(_data);
			Node * next = it.p;
			p->next = next;
			if(next)
			{
				p->prev = next->prev;
				next->prev = p;
			}
			else
			{
				p->prev = tail;
				tail = p;
			}

			if(p->prev)
			{
				p->prev->next = p;
			}
			else
			{
				head = p;
			}
#ifdef NB_DEBUG
			return iterator(p, this);
#else
			return iterator(p);
#endif
		}

		void clear()
		{
			Node * p;
			Node * p1;
			p = head;
			while(p)
			{
				p1 = p;
				p = p->next;
				nd_free(p1);
			}
			head = 0;
			tail = 0;
		}

		T & front()
		{ return head->data; }
		T & back()
		{ return tail->data; }
	};

}

