
#ifndef NB_CORE_ARRAY_H
#define NB_CORE_ARRAY_H

#include <new>
#include <nbug/core/debug.h>

namespace e
{
//#define E_ASSERT(x) ((void*)0)
	template <typename T> class Array
	{
		T * buf;
		size_t len;
		size_t buf_size;
	public:
		Array()
		{
			buf = 0;
			len = 0;
			buf_size = 0;
		}

		Array(size_t _size, const T & _v = T())
		{
			buf = 0;
			len = 0;
			buf_size = 0;
			resize(_size, _v);
		}

		Array(const Array & _r)
		{
			len = _r.len;
			buf_size = len;
			buf = (T*)nd_malloc(len * sizeof(T));
			for(size_t i = 0; i < len; i++)
			{
				new(buf + i) T(_r.buf[i]);
			}
		}

		const Array & operator=(const Array & _r)
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

		~Array()
		{
			clear();
		}

		size_t size() const
		{
			return len;
		}

		void resize(size_t _size, const T & _v = T())
		{
			//stao->resize(_size);
			if(len < _size)
			{
				if(_size >= buf_size)
				{
					buf_size = _size + 1 + (_size >> 1);
					buf = (T*)nd_realloc(buf, buf_size * sizeof(T));
				}
				for(size_t i = len; i < _size; i++)
				{
					new(buf+i) T(_v);
				}

			}
			else
			{
				for(size_t i = _size; i < len; i++)
				{
					(buf + i)->~T();
				}
			}
			len = _size;
		}

		void remove(size_t _index)
		{
			E_ASSERT(_index < len);
			for(size_t i=_index; i<len-1; i++)
			{
				buf[i] = buf[i+1];
			}
			len--;
			buf[len].~T();
		}

		void insert(size_t _index, const T & _d)
		{
			if(_index <= (int)len)
			{
				resize(len + 1, T());
				for(size_t i = len - 1; i > _index; i--)
				{
					buf[i] = buf[i-1];
				}
				buf[_index] = _d;
			}
			else
			{
				push_back(_d);
			}
		}

		T & operator[](size_t _index)
		{ E_ASSERT(len > _index); return buf[_index]; }

		const T & operator[](size_t _index) const
		{ E_ASSERT(len > _index); return buf[_index]; }

		T & at(size_t _index)
		{ E_ASSERT(len > _index); return buf[_index]; }

		const T & at(size_t _index) const
		{ E_ASSERT(len > _index); return buf[_index]; }

		T & front()
		{ E_ASSERT(len); return buf[0]; }

		const T & front() const
		{ E_ASSERT(len); return buf[0]; }

		T & back()
		{ E_ASSERT(len); return buf[len - 1]; }

		const T & back() const
		{ E_ASSERT(len); return buf[len - 1]; }

		void push_back(const T & _s)
		{
			len++;
			if(len >= buf_size)
			{
				buf_size = len + 1 + (len >> 1);
				buf = (T*)nd_realloc(buf, buf_size * sizeof(T));
			}
			new(buf+len-1) T(_s);
		}

		void pop_back()
		{
			E_ASSERT(len > 0);
			len--;
			(buf+len)->~T();
		}

		bool empty() const
		{
			return len == 0;
		}

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

		void swap(Array & _r)
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
