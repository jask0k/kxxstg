
#ifndef NB_CORE_STACK_H
#define NB_CORE_STACK_H

#include <new>
#include <nbug/core/debug.h>
//#include <nbug/core/nd_memory.h>

namespace e
{
	template <typename T> class Stack
	{
		T * buf;
		size_t len;
		size_t buf_size;
	public:
		Stack()
		{
			buf = 0;
			len = 0;
			buf_size = 0;
		}

		Stack(size_t _size, const T & _v = T())
		{
			buf = 0;
			len = 0;
			buf_size = 0;
			resize(_size, _v);
		}

		Stack(const Stack & _r)
		{
			len = _r.len;
			buf_size = len;
			buf = (T*)nd_malloc(len * sizeof(T));
			for(size_t i = 0; i < len; i++)
			{ new(buf + i) T(_r.buf[i]); }
		}

		const Stack & operator=(const Stack & _r)
		{
			if(this != & _r)
			{
				// TODO: optimize
				clear();
				len = _r.len;
				buf_size = len;
				buf = (T*)nd_malloc(len * sizeof(T));
				for(size_t i = 0; i < len; i++)
				{
					new(buf + i) T(_r.buf[i]);
				}
			}
			return *this;
		}

		~Stack()
		{ clear(); }

		T & top()
		{ E_ASSERT(len > 0); return buf[len - 1]; }

		const T & top() const
		{ E_ASSERT(len > 0); return buf[len - 1]; }

		void push(const T & _s)
		{
			len++;
			if(len >= buf_size)
			{
				buf_size = len + 1 + (len >> 1);
				buf = (T*)nd_realloc(buf, buf_size * sizeof(T));
			}
			new(buf+len-1) T(_s);
		}

		void pop()
		{
			E_ASSERT(len > 0);
			len--;
			(buf+len)->~T();
		}

		bool empty() const
		{ return len == 0; }

		void clear()
		{
			for(size_t i = 0; i < len; i++)
			{
				(buf + i)->~T();
			}
			nd_free(buf);
			buf = 0;
			len = 0;
			buf_size = 0;
		}

		void swap(Stack & _r)
		{
			T * p = buf;
			buf = _r.buf;
			_r.buf = p;

			size_t t = len;
			len = _r.len;
			_r.len = t;

			t = buf_size;
			buf_size = _r.buf_size;
			_r.buf_size = t;
		}
	};
}

#endif
