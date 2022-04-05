
#pragma once

#include <string.h>
#include <nbug/core/debug.h>

namespace e
{
	class Buffer
	{
	protected:
		unsigned char * buf;
		size_t len;
	public:

		Buffer(size_t _len = 0)
		{
			len = _len;
			buf = (unsigned char*) malloc(_len);
		}

		Buffer(const Buffer & _r)
		{
			len = _r.len;
			buf = (unsigned char*) malloc(_r.len);
			memcpy(buf, _r.buf, len);
		}

		~Buffer()
		{ free(buf); }

		size_t size() const
		{ return len; }

		void resize(size_t _len)
		{
			free(buf);
			len = _len;
			buf = (unsigned char*) malloc(_len);
		}

		operator unsigned char *()
		{ return buf; }

		operator const unsigned char *() const
		{ return buf; } 

		bool operator==(const Buffer & _r)
		{ return len == _r.len && memcmp(buf, _r.buf, len) == 0; }

		const Buffer & operator=(const Buffer & _r)
		{
			if(this != &_r)
			{
				free(buf);
				len = _r.len;
				buf = (unsigned char*) malloc(_r.len);
				memcpy(buf, _r.buf, len);
			}
			return *this;
		}
	};
}
