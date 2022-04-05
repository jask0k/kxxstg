#pragma once

#include <new>
#include <nbug/core/debug.h>

namespace e
{
	template <typename T> class Array2
	{
		T * buf;
		unsigned int colCount;
		unsigned int rowCount;
		unsigned int len;
		unsigned int bufSize;
	public:
		Array2()
		{
			buf = 0;
			colCount = 0;
			rowCount = 0;
			len = 0;
			bufSize = 0;
		}
		Array2(size_t _rows, size_t _cols, const T & _v = T())
		{
			buf = 0;
			colCount = 0;
			rowCount = 0;
			len = 0;
			bufSize = 0;
			resize(_rows, _cols, _v);
		}
		Array2(const Array2 & _r)
		{
			colCount = _r.colCount;
			rowCount = _r.rowCount;
			len = _r.len;
			bufSize = len;
			buf = (T*)nd_malloc(len * sizeof(T));
			for(size_t i = 0; i < len; i++)
			{
				new(buf + i) T(_r.buf[i]);
			}
		}

		const Array2 & operator=(const Array2 & _r)
		{
			if(this != & _r)
			{
				// TODO: optimize
				clear();
				colCount = _r.colCount;
				rowCount = _r.rowCount;
				bufSize = len;
				buf = (T*)nd_malloc(len * sizeof(T));
				for(size_t i = 0; i < len; i++)
				{
					new(buf + i) T(_r.buf[i]);
				}
			}
			return *this;
		}

		~Array2()
		{
			clear();
		}
		
		void clear()
		{
			for(size_t i = 0; i < len; i++)
			{
				(buf + i)->~T();
			}
			nd_free(buf);
			buf = 0;
			colCount = 0;
			rowCount = 0;
			len = 0;
			bufSize = 0;
		}

		void resize(size_t _rows, size_t _cols, const T & _v = T())
		{
			size_t newLen = _rows * _cols;
			if(len < newLen)
			{
				if(newLen >= bufSize)
				{
					bufSize = newLen + 1 + (newLen >> 1);
					buf = (T*)nd_realloc(buf, bufSize * sizeof(T));
				}
				for(size_t i = len; i < newLen; i++)
				{
					new(buf+i) T(_v);
				}

			}
			else
			{
				for(size_t i = newLen; i < len; i++)
				{
					(buf + i)->~T();
				}
			}
			len = newLen;
			colCount = _cols;
			rowCount = _rows;
		}

		T * operator[](int _row)
		{ E_ASSERT(_row >= 0 && _row < rowCount); return buf + _row * colCount; }

		const T * operator[](int _row) const
		{ E_ASSERT(_row >= 0 && _row < rowCount); return buf + _row * colCount; }

		T & operator()(size_t _row, size_t _col)
		{ E_ASSERT(_col >= 0 && _col < colCount && _row >= 0 && _row < rowCount); return buf[_col + _row * colCount]; }

		const T & operator()(size_t _row, size_t _col) const
		{ E_ASSERT(_col >= 0 && _col < colCount && _row >= 0 && _row < rowCount); return buf[_col + _row * colCount]; }
		
		size_t rows() const
		{ return rowCount; }
		
		size_t cols() const
		{ return colCount; }
	};
}
