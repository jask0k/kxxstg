
//#include "private.h"
#include <locale.h>
#include <nbug/core/debug_.h>
#include <ctype.h>
#include <wctype.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <nbug/core/str.h>
#include <nbug/core/debug.h>
#include <nbug/core/missing.h>

// stringa and stringw is used within debug memory funcions
// so debug memory functions can not be use within stringa and stringw
// we must undef relative macros here.
#undef malloc
#undef realloc
#undef calloc
#undef free

// warning: don't use operator enew to alloc memory for stringw and stringa
namespace e
{
	static bool g_loacale_inited = false;
	static inline void InitLocale()
	{
		const char * locale_string = setlocale(LC_CTYPE, "");
		g_loacale_inited = true;
//		if(locale_string[0]==0 || strcmp("C", locale_string)==0)
//		{
//			setlocale(LC_CTYPE, "zh_CN");
//			locale_string = setlocale(LC_CTYPE, 0);
//		}
		message(L"[nb] LC_CTYPE = \"" + stringw(locale_string) + L"\"");
	}

	//template <typename T>
	//static inline void IntToString(T * _buf, int _i)
	//{
	//	if(_i == 0)
	//	{
	//		_buf[0] = '0';
	//		_buf[1] = 0;
	//		return;
	//	}
	//	T * p = _buf;
	//	if(_i < 0)
	//	{
	//		*p++ = L'-';
	//		_i=-_i;
	//	}

	//	T buf1[60];
	//	T * p1 = buf1;
	//	while(_i)
	//	{
	//		*p1++ = (_i % 10) + L'0';
	//		_i/= 10;
	//	}
	//	while(p1 != buf1)
	//	{
	//		p1--;
	//		*p++ = *p1;
	//	}

	//	*p = 0;
	//}


	template <typename T>
	static inline int icompare(const T * _a, const T * _b)
	{
		//E_ASSERT(L'A') < _T('a');

		T left,right;
		while(*_a)
		{
			left = *_a++;
			if(left >= 'A' && left <= 'Z')
			{
				left+= (int)'a' - (int)'A';
			}
			right = *_b++;
			if(right >= 'A' && right <= 'Z')
			{
				right+= (int)'a' - (int)'A';
			}
			int i = left - right;
			if(i != 0)
			{
				return i;
			}
		}
		return *_b ? -1 : 0;
	}

	stringa::stringa()
	{
		buf_size = 1;
		buf = (char*) malloc(1);
		buf[0] = 0;
	}

	static inline int wsctombs_len(const wchar_t * _r)
	{
		if(!g_loacale_inited)
		{
			InitLocale();
		}
		int ret = 0;
		char buf[10];
		while(*_r != 0)
		{
#if defined(NB_WINDOWS) && defined(_MSC_VER)
			int n;
			if(wctomb_s(&n, buf, sizeof(buf), *_r) != 0)
			{
				return -1;
			}
#else
			size_t n = wctomb(buf, *_r);
			if(n == (size_t)-1)
			{
				return -1;
			}
#endif
			ret+=n;
			_r++;
		}
		return ret;
	}

	void stringa::init(const wchar_t * _r, Charset _dstCharset)
	{
	    if(_r == 0)
		{
			buf_size = 1;
			buf = (char*) malloc(1);
			buf[0] = 0;
			return;
		}
		switch(_dstCharset)
		{
		case CHARSET_LOCALE:
			{
				buf_size = wsctombs_len(_r) + 1;
				if(buf_size == 0)
				{
#ifdef NB_CFG_VERBOSE
					E_TRACE_LINE(L"[nb] (WW) stringa:Init() Convert failed Unicode => Locale.");
#endif
					buf_size = wsctombs_len(_r) + 1;
					buf_size = 1;
					buf =  (char*)malloc(1);
					buf[0] = 0;
					return;
				}
				buf = (char*) malloc(buf_size);
				size_t len = wcstombs(buf,  _r,  buf_size);
				if(len == (size_t)-1)
				{
#ifdef NB_CFG_VERBOSE
					E_TRACE_LINE(L"[nb] (WW) stringa:Init() Convert failed: Unicode => Locale.");
#endif
					buf[0] = 0;
				}
			}
			break;
		case CHARSET_UTF8:
			{
				/*
				U-00000000 - U-0000007F:  0xxxxxxx
				U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
				U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
				U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
				U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
				*/
				const wchar_t * p = _r;
				int charCount = 0;
				while(*p)
				{
					uint32 ch = *p;
					if(ch <= 0x0000007F)
					{
						charCount+= 1;
					}
					else if(ch <= 0x000007FF)
					{
						charCount+= 2;
					}
					else if(ch <= 0x0000FFFF)
					{
						charCount+= 3;
					}
					else if(ch <= 0x001FFFFF)
					{
						charCount+= 4;
					}
					else if(ch <= 0x03FFFFFF)
					{
						charCount+= 5;
					}
					else if(ch <= 0x7FFFFFFF)
					{
						charCount+= 6;
					}
					else
					{
#ifdef NB_CFG_VERBOSE
						E_TRACE_LINE(L"([NB] (WW) stringa:Init() This char can not convert to UTF-8: ch = " + string((int)(ch)));
#endif
						charCount = -1;
						break;
					}
					p++;
				}

				if(charCount < 0)
				{
#ifdef NB_CFG_VERBOSE
					E_TRACE_LINE(L"[nb] (WW) stringa:Init() Convert failed: Unicode => UTF-8.");
#endif
					buf_size = 1;
					buf =  (char*)malloc(1);
					buf[0] = 0;
					return;
				}
				buf_size = charCount + 1;
				buf = (char*) malloc(buf_size);
				p = _r;
				char * p1 = buf;
				while(*p)
				{
				/*
				U-00000000 - U-0000007F:  0xxxxxxx
				U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
				U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
				U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
				U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

				*/
					uint32 ch = *p;
					if(ch <= 0x0000007F)
					{
						p1[0] = ch & 0x7f;
						p1++;
					}
					else if(ch <= 0x000007FF)
					{
						p1[0] = ((ch >> 6) & 0x1f) | 0xc0;
						p1[1] = ((ch >> 0) & 0x3f) | 0x80;
						p1+= 2;
					}
					else if(ch <= 0x0000FFFF)
					{
						p1[0] = ((ch >> 12) & 0x0f) | 0xe0;
						p1[1] = ((ch >> 6) & 0x3f) | 0x80;
						p1[2] = ((ch >> 0) & 0x3f) | 0x80;
						p1+= 3;
					}
					else if(ch <= 0x001FFFFF)
					{
						p1[0] = ((ch >> 18) & 0x07) | 0xf0;
						p1[1] = ((ch >> 12) & 0x3f) | 0x80;
						p1[2] = ((ch >> 6) & 0x3f) | 0x80;
						p1[3] = ((ch >> 0) & 0x3f) | 0x80;
						p1+= 4;
					}
					else if(ch <= 0x03FFFFFF)
					{
						p1[0] = ((ch >> 24) & 0x03) | 0xf8;
						p1[1] = ((ch >> 18) & 0x3f) | 0x80;
						p1[2] = ((ch >> 12) & 0x3f) | 0x80;
						p1[3] = ((ch >> 6) & 0x3f) | 0x80;
						p1[4] = ((ch >> 0) & 0x3f) | 0x80;
						p1+= 5;
					}
					else if(ch <= 0x7FFFFFFF)
					{
						p1[0] = ((ch >> 30) & 0x01) | 0xfc;
						p1[1] = ((ch >> 24) & 0x3f) | 0x80;
						p1[2] = ((ch >> 18) & 0x3f) | 0x80;
						p1[3] = ((ch >> 12) & 0x3f) | 0x80;
						p1[4] = ((ch >> 6) & 0x3f) | 0x80;
						p1[5] = ((ch >> 0) & 0x3f) | 0x80;
						p1+= 6;
					}
					else
					{
						E_ASSERT(0);
					}
					p++;
				}
				*p1 = 0;

			}
			break;
		default:
			{
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE(L"([NB] (WW) stringa:Init() Unsupported Charset.");
#endif
				buf_size = wcslen(_r) + 1;
				buf = (char*) malloc(buf_size * sizeof(char));
				char * p = buf;
				do
				{
					*p++= (char)(*_r);
				}while(*_r++);
			}
			break;
		}
	}


	stringa::stringa(const wchar_t * _r, Charset _dstCharset)
	{
		init(_r, _dstCharset);
	}

	stringa::stringa(const stringw & _r, Charset _dstCharset)
	{
		init(_r.c_str(), _dstCharset);
	}

	stringa::stringa(const char * _str_init)
	{
	    if(_str_init == 0)
		{
			buf_size = 1;
			buf = (char*) malloc(1);
			buf[0] = 0;
			return;
		}

		buf_size = strlen(_str_init) + 1;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, _str_init);
	}

	stringa::stringa(const stringa & _r)
	{
		buf_size = strlen(_r.buf) + 1;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, _r.buf);
	}

	const stringa & stringa::operator=(const stringa & _r)
	{
		if(this != &_r)
		{
			reserve(strlen(_r.buf) + 1);
			strcpy_s(buf, buf_size, _r.buf);
		}
		return *this;
	}


	stringa::stringa(int _r)
	{
		char buf1[60];
		sprintf(buf1, "%d", _r);
		buf_size = strlen(buf1) + 1;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, buf1);
	}

	stringa::stringa(uint _r)
	{
		char buf1[60];
		sprintf(buf1, "%u", _r);
		buf_size = strlen(buf1) + 1;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, buf1);
	}

	stringa::stringa(double _r, int _digits)
	{
		char fmt[32];
		sprintf(fmt, "%%.%dlf", _digits);
		char buf1[60];
		sprintf(buf1, fmt, _r);
		buf_size = strlen(buf1) + 1;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, buf1);
	}

	stringa::stringa(float _r, int _digits)
	{
		char fmt[32];
		sprintf(fmt, "%%.%df", _digits);
		char buf1[60];
		sprintf(buf1, fmt, _r);
		buf_size = strlen(buf1) + 1;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, buf1);
	}

	stringa::stringa(char _r)
	{
		buf_size = 2;
		buf = (char*) malloc(buf_size);
		buf[0] = _r;
		buf[1] = 0;
	}

	stringa::stringa(bool _r)
	{
		buf_size = 6;
		buf = (char*) malloc(buf_size);
		strcpy_s(buf, buf_size, _r ? "true" : "false");
	}

	//stringa::stringa(unsigned char _r)
	//{
	//	buf_size = 3;
	//	buf = (char*) malloc(buf_size);
	//	int i;
	//	i = (_r >> 4) & 0x0f;
	//	buf[0] = i > 9 ? 'A' + i - 10 : '0' + i;
	//	i= _r & 0x0f;
	//	buf[1] = i > 9 ? 'A' + i - 10 : '0' + i;
	//	buf[2] = 0;
	//}

	stringa::~stringa()
	{
		free(buf);
	}

	char * stringa::c_str() const
	{
		return buf;
	}

	int stringa::length() const
	{
		return strlen(buf);
	}

	void stringa::reserve(int _capacity)
	{
		if(_capacity < length())
		{
			_capacity = length();
		}

		if(buf_size - 1 < _capacity)
		{
			buf_size = _capacity + 1;
			buf = (char*) realloc(buf, buf_size);
		}
	}

	//int stringa::Capacity() const
	//{
	//	return buf_size - 1;
	//}

	char stringa::operator[](int _index) const
	{
	    E_ASSERT(_index < buf_size);
		return buf[_index];
	}

	char & stringa::operator[](int _index)
	{
        E_ASSERT(_index < buf_size);
		return buf[_index];
	}

	int stringa::compare(const stringa & _r) const
	{
		return strcmp(buf, _r.buf);
	}

	int stringa::icompare(const stringa & _r) const
	{
		return e::icompare<char>(buf, _r.buf);
	}

	bool stringa::operator==(const stringa & _r) const
	{
		return compare(_r) == 0;
	}

	bool stringa::operator!=(const stringa & _r) const
	{
		return compare(_r) != 0;
	}

	bool stringa::operator<(const stringa & _r) const
	{
		return compare(_r) < 0;
	}

	//bool stringa::operator<=(const stringa & _r) const
	//{
	//	return compare(_r) <= 0;
	//}

	bool stringa::operator>(const stringa & _r) const
	{
		return compare(_r) > 0;
	}

	//bool stringa::operator>=(const stringa & _r) const
	//{
	//	return compare(_r) >= 0;
	//}

	int stringa::to_int() const
	{
		return atoi(buf);
	}

	//uint stringa::ToUint() const
	//{
	//	return static_cast<uint >(atoi(buf));
	//}

	float stringa::to_float() const
	{
		return (float) atof(buf);
	}

	bool stringa::to_bool() const
	{
		if(length() == 0)
		{
			return false;
		}

		char ch = buf[0];
		return ch == 'T' || ch == 't' || ch == '1';
	}

	double stringa::to_double() const
	{
		return atof(buf);
	}

	stringa stringa::operator+(const stringa & _r) const
	{
		//stringa * pThis = const_cast<stringa *>(this);
		stringa s(*this);
		s.reserve(s.length() + _r.length());
		strcat_s(s.buf, s.buf_size, _r.buf);
		return s;
	}

	void stringa::append(const stringa & _r)
	{
		reserve(length() + _r.length());
		strcat_s(buf, buf_size, _r.buf);
	}

	const stringa & stringa::operator+=(const stringa & _r)
	{
		reserve(length() + _r.length());
		strcat_s(buf, buf_size, _r.buf);
		return *this;
	}

	//int stringa::Find(char _ch) const
	//{
	//	char * p = strchr(buf, _ch);
	//	if(p != NULL)
	//	{
	//		return (int)(p - buf);
	//	}
	//	else
	//	{
	//		return -1;
	//	}
	//}

	//int stringa::Find(const char * _chs) const
	//{
	//	char * p = NULL;
	//	while(*_chs)
	//	{
	//		char * p1 = strchr(buf, *_chs);
	//		p = p1 == NULL ? p : (p > p1 ? p : p1);
	//		_chs++;
	//	}

	//	if(p != NULL)
	//	{
	//		return (int)(p - buf);
	//	}
	//	else
	//	{
	//		return -1;
	//	}
	//}

	int stringa::replace(const char * _from, const char * _to)
	{
		int lenFom = strlen(_from);
		int lenTo  = strlen(_to);
		int delta  = lenTo - lenFom;
		int count  = 0;
		const char * p;
		for(p = buf; *p;)
		{
			if(memcmp(p, _from, lenFom * sizeof(char)) == 0)
			{
				count ++;
				p+= lenFom;
			}
			else
			{
				p++;
			}
		}

		size_t newSize = buf_size + delta * count;
		char * newBuf = (char *) malloc(sizeof(char) * newSize);
			memset(newBuf, 0, sizeof(char) * newSize);
		char *p1 = newBuf;
		for(p = buf; *p;)
		{
			if(memcmp(p, _from, lenFom * sizeof(char)) == 0)
			{
				memcpy(p1, _to, lenTo * sizeof(char));
				p1+=lenTo;
				p+= lenFom;
			}
			else
			{
				*p1=*p;
				p++;
				p1++;
			}
		}

		free(buf);
		buf = newBuf;
		buf_size = newSize;

		return count;
	}

	int stringa::replace(const char _from, const char _to)
	{
		int ret = 0;
		char * p = buf;
		while(*p)
		{
			if(*p == _from)
			{
				*p = _to;
				ret++;
			}
			*p++;
		}
		return ret;
	}

	stringa stringa::substr(int _from, int _len) const
	{
		int totalLen = (int)length();
		if(_from >= totalLen)
		{
			return "";
		}
		if(_len == -1 || _from + _len > totalLen)
		{
			_len = totalLen - _from;
		}

		stringa ret;
		ret.reserve(_len);
		memcpy(ret.buf, buf + _from, _len * sizeof(char));
		ret.buf[_len] = 0;
		return ret;
	}

	//void stringa::Delete(int _from, int _len)
	//{
	//	int totalLen = (int)length();
	//	if(_from >= totalLen)
	//	{
	//		return;
	//	}
	//	if(_len == -1 || _from + _len > totalLen)
	//	{
	//		_len = totalLen - _from;
	//	}
	//	*this = substr(0, _from) + substr(_from + _len, -1);
	//}

	int stringa::find(char _ch, int _pos) const
	{
		E_ASSERT(_pos >= 0);
		if(_pos >= 0 && _pos < length())
		{
			char * p = strchr(buf + _pos, _ch);
			if(p != NULL)
			{
				return (int)(p - buf);
			}
		}
		return -1;
	}

	int stringa::find_str(const char * _s, int _pos) const
	{
		E_ASSERT(_pos >= 0);
		if(_pos >= 0 && _pos < length())
		{
			const char * p = buf + _pos;
			const char * p1 = strstr(p, _s);
			if(p1)
			{
				return _pos + (p1 - p);
			}
		}

		return -1;
	}

	int stringa::rfind(char _ch, int _pos) const
	{
		E_ASSERT(_pos >= 0 || _pos == -1);
		if(_pos == -1)
		{
			_pos = length();
		}

		for(;_pos >=0; _pos--)
		{
			if(buf[_pos] == _ch)
			{
				return _pos;
			}
		}
		return -1;
	}

	int stringa::rfind_any(const char * _chs, int _pos) const
	{
		E_ASSERT(_pos >= 0 || _pos == -1);
		if(_pos == -1)
		{
			_pos = length();
		}

		for(;_pos >=0; _pos--)
		{
			const char * p = _chs;
			while(*p)
			{
				if(buf[_pos] == *p++)
				{
					return _pos;
				}
			}
		}
		return -1;
	}


	int stringa::find_any(const char * _chs, int _pos) const
	{
		E_ASSERT(_pos >= 0);
		if(_pos < length())
		{
			char * p = NULL;
			while(*_chs)
			{
				char * p1 = strchr(buf + _pos, *_chs);
				p = p1 == NULL ? p : (p < p1 && p != NULL ? p : p1);
				_chs++;
			}
			if(p != NULL)
			{
				return (int)(p - buf);
			}
		}
		return -1;
	}

	//Array<stringa> stringa::Split(const char * _chs, int _from, int _n) const
	//{
	//	Array<stringa> ret;
	//	int len = length();
	//	if(_n!= -1 && _from + _n < len)
	//	{
	//		len = _from + _n;
	//	}
	//	int pos = _from;
	//	while(pos < len)
	//	{
	//		int n = find(_chs, pos);
	//		if(n == -1)
	//		{
	//			stringa t = substr(pos, -1);
	//			if(!t.empty())
	//			{
	//				ret.push_back(t);
	//				break;
	//			}
	//		}
	//		else
	//		{
	//			stringa t = substr(pos, n - pos);
	//			if(!t.empty())
	//			{
	//				ret.push_back(t);
	//			}
	//			pos = n + 1;
	//		}
	//	}
	//	return ret;
	//}

	bool stringa::empty() const
	{
		return length() == 0;
	}

	void stringa::clear()
	{
		free(buf);
		buf_size = 1;
		buf = (char*) malloc(1);
		buf[0] = 0;
	}

	void stringa::append(char _ch)
	{
		int len = length() ;
		reserve(len + 1);
		buf[len++] = _ch;
		buf[len] = 0;
	}

	void stringa::insert(int _index, const stringa & _r)
	{
		if(_r.empty())
		{
			return;
		}
		int len = length();
		E_BASIC_ASSERT(_index >= 0 && _index <= len);
		int len1 = _r.length();
		reserve(len + len1);
		char * p0 = buf + len;
		char * pe = buf + _index;
		char * p1 = buf + len + len1;
		while(p0 >= pe)
		{
			*p1-- = *p0--;
		}
		memcpy(pe, _r.buf, len1 * sizeof(char));
	}
	void stringa::insert(int _index, char _ch)
	{
		int len = length();
		E_BASIC_ASSERT(_index >= 0 && _index <= len);
		reserve(len + 1);
		for(int i = len + 1; i > _index; i--)
		{
			buf[i] = buf[i-1];
		}
		buf[_index] = _ch;
	}

	//int stringa::HexDecode(void * _bufOut, int _size) const
	//{
	//	E_ASSERT(_size >= 0);
	//	unsigned char * _out = (unsigned char*)_bufOut;
	//	int len = length();
	//	int n = 0;
	//	for(int i=0; i<_size; i++)
	//	{
	//		int j = i + i;
	//		if(j+1 < len)
	//		{
	//			unsigned char b0, b1;
	//			char ch = buf[j];
	//			if(ch >='0' && ch <='9')
	//			{
	//				b0 = ch - '0';
	//			}
	//			else if(ch >='A' && ch <='F')
	//			{
	//				b0 = ch - 'A' + 10;
	//			}
	//			else if(ch >='a' && ch <='f')
	//			{
	//				b0 = ch - 'a' + 10;
	//			}
	//			else
	//			{
	//				_out[i] = 0;
	//				n++;
	//				continue;
	//			}
	//			ch = buf[j+1];
	//			if(ch >='0' && ch <='9')
	//			{
	//				b1 = ch - '0';
	//			}
	//			else if(ch >='A' && ch <='F')
	//			{
	//				b1 = ch - 'A' + 10;
	//			}
	//			else if(ch >='a' && ch <='f')
	//			{
	//				b1 = ch - 'a' + 10;
	//			}
	//			else
	//			{
	//				_out[i] = 0;
	//				n++;
	//				continue;
	//			}

	//			_out[i] = (b0  << 4) | b1;
	//			n++;
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}

	//	return n;
	//}

	//void stringa::HexEncode(const void * _bufIn, int _size)
	//{
	//	E_ASSERT(_size >= 0);
	//	unsigned char * _in = (unsigned char*)_bufIn;
	//	reserve(_size * 2);
	//	for(int i=0; i<_size; i++)
	//	{
	//		int j = i + i;
	//		unsigned char b0, b1;
	//		b0 = (_in[i] >> 4) & 0x0f;
	//		b1 = _in[i] & 0x0f;
	//		if(b0 > 9)
	//		{
	//			buf[j] = b0 - 10 + 'A';
	//		}
	//		else
	//		{
	//			buf[j] = b0 + '0';
	//		}

	//		if(b1 > 9)
	//		{
	//			buf[j+1] = b1 - 10 + 'A';
	//		}
	//		else
	//		{
	//			buf[j+1] = b1 + '0';
	//		}

	//	}
	//	buf[_size * 2] = 0;
	//}

	stringa stringa::format(const char * _format, va_list _argptr)
	{
		stringa s;
#ifdef _MSC_VER
		int bufSize = _vscprintf(_format, _argptr) + 1;
		s.reserve(bufSize);
		vsprintf_s(s.c_str(), bufSize, _format, _argptr);
#else
		int bufSize = vsnprintf(0, 0, _format, _argptr) + 1;
		s.reserve(bufSize);
		vsnprintf(s.c_str(), bufSize, _format, _argptr);
#endif
		return s;
	}

	stringa stringa::format(const char * _format, ...)
	{
		va_list argptr;
		va_start(argptr, _format);
		stringa s = format(_format, argptr);
		va_end(argptr);
		return s;
	}

	void stringa::trim()
	{
		char * a = buf;
		char * b = buf + strlen(buf) - 1;
		while(*a && isspace(*a)) a++;
		while(b >= a && isspace(*b)) b--;
		int len = b - a + 1;
		if(len > 0)
		{
			memmove(buf, a, len);
			buf[len] = 0;
		}
		else
		{
			buf[0] = 0;
		}
	}

	//void stringa::TrimStart()
	//{
	//	char * a = buf;
	//	char * b = buf + strlen(buf) - 1;
	//	while(*a && isspace(*a)) a++;
	//	//while(b >= a && isspace(*b)) b--;
	//	int len = b - a + 1;
	//	if(len > 0)
	//	{
	//		memmove(buf, a, len);
	//		buf[len] = 0;
	//	}
	//	else
	//	{
	//		buf[0] = 0;
	//	}
	//}

	//void stringa::TrimEnd()
	//{
	//	char * a = buf;
	//	char * b = buf + strlen(buf) - 1;
	//	//while(*a && isspace(*a)) a++;
	//	while(b >= a && isspace(*b)) b--;
	//	int len = b - a + 1;
	//	if(len > 0)
	//	{
	//		memmove(buf, a, len);
	//		buf[len] = 0;
	//	}
	//	else
	//	{
	//		buf[0] = 0;
	//	}
	//}

//	void stringa::trim(const char * _chs)
//	{
//		char * a = buf;
//		char * b = buf + strlen(buf) - 1;
//		while(*a && strchr(_chs, *a)) a++;
//		while(b >= a && strchr(_chs, *b)) b--;
//		int len = b - a + 1;
//		if(len > 0)
//		{
//			memmove(buf, a, len);
//			buf[len] = 0;
//		}
//		else
//		{
//			buf[0] = 0;
//		}
//	}

	//void stringa::TrimStart(const char * _chs)
	//{
	//	char * a = buf;
	//	char * b = buf + strlen(buf) - 1;
	//	while(*a && strchr(_chs, *a)) a++;
	//	//while(b >= a && strchr(_chs, *b)) b--;
	//	int len = b - a + 1;
	//	if(len > 0)
	//	{
	//		memmove(buf, a, len);
	//		buf[len] = 0;
	//	}
	//	else
	//	{
	//		buf[0] = 0;
	//	}
	//}

	//void stringa::TrimEnd(const char * _chs)
	//{
	//	char * a = buf;
	//	char * b = buf + strlen(buf) - 1;
	//	//while(*a && strchr(_chs, *a)) a++;
	//	while(b >= a && strchr(_chs, *b)) b--;
	//	int len = b - a + 1;
	//	if(len > 0)
	//	{
	//		memmove(buf, a, len);
	//		buf[len] = 0;
	//	}
	//	else
	//	{
	//		buf[0] = 0;
	//	}
	//}

	void stringa::to_upper()
	{
		char * p = buf;
		while(*p)
		{
			*p = toupper(*p);
			p++;
		}
	}

	void stringa::to_lower()
	{
		char * p = buf;
		while(*p)
		{
			*p = tolower(*p);
			p++;
		}
	}

	//stringa stringa::ExtractFirstWord(const char * _chs, bool _erase)
	//{
	//	stringa ret;
	//	int n = find(_chs);
	//	if(n == -1)
	//	{
	//		ret = *this;
	//		this->clear();
	//	}
	//	else
	//	{
	//		ret = substr(0, n-1);
	//		if(_erase)
	//		{
	//			*this = substr(n+1, -1);
	//		}
	//	}
	//	return ret;
	//}
}


namespace e
{
	stringw::stringw()
	{
		buf_size = 1;
		buf = (wchar_t*) malloc(1 * sizeof(wchar_t));
		buf[0] = 0;
	}

	static inline int mbstowcs_len(const char * _r)
	{
		if(!g_loacale_inited)
		{
			InitLocale();
		}

		size_t len = strlen(_r);
		int ret = 0;
		wchar_t buf[10];
		while(*_r != 0)
		{
			size_t n = mbtowc(buf, _r, len);
			if(n == (size_t)-1)
			{
				return -1;
			}
			ret++;
			_r+= n;
			len-=n;
		}

		return ret;
	}

	void stringw::init(const char * _r, Charset _srcCharset)
	{
		if(_r == 0)
		{
			buf_size = 1;
			buf = (wchar_t*) malloc(1 * sizeof(wchar_t));
			buf[0] = 0;
			return;
		}
		switch(_srcCharset)
		{
		case CHARSET_LOCALE:
			{
				buf_size = mbstowcs_len(_r) + 1;

				if(buf_size)
				{
					size_t buf_size_in_byte = buf_size * sizeof(wchar_t);
					buf = (wchar_t*) malloc(buf_size_in_byte);
					size_t len = mbstowcs(buf,  _r,  buf_size_in_byte);
					if(len == (size_t)-1)
					{
#ifdef NB_CFG_VERBOSE
						E_TRACE(L"[nb] (WW) Locale => Unicode failed");
#endif
						free(buf);
						goto FALLBACK;
					}
				}
				else
				{
#ifdef NB_CFG_VERBOSE
					E_TRACE(L"[nb] (WW) Locale => Unicode failed");
#endif
					goto FALLBACK;
				}
			}
			return;
		case CHARSET_UTF8:
			{
				// calc the len
				const unsigned char * p = (unsigned char *)_r;
				int charCount = 0;
				while(*p != 0)
				{
					unsigned char ch = *p;
					if((ch & 0xf8) == 0xf0)
					{
						// 4 bytes
						p+= 4;
					}
					else  if((ch & 0xf0) == 0xe0)
					{
						// 3 bytes
						p+= 3;
					}
					else if((ch & 0xe0) == 0xc0)
					{
						// 2 bytes
						p+= 2;
					}
					else if((ch & 0x80) == 0)
					{
						// 1 byte
						p+= 1;
					}
					else
					{
#ifdef NB_CFG_VERBOSE
						E_TRACE(L"[nb] (WW) UTF8 => Unicode failed");
#endif
						//buf_size = 1;
						//buf = (wchar_t*) malloc(1 * sizeof(wchar_t));
						//buf[0] = 0;
						//return;
						goto FALLBACK;
					}
					charCount++;
				}
				buf_size = charCount + 1;
				size_t buf_size_in_byte = buf_size * sizeof(wchar_t);
				buf = (wchar_t*) malloc(buf_size_in_byte);
				p = (unsigned char *)_r;
				wchar_t * p1 = buf;
				while(*p != 0)
				{
					unsigned char ch = *p;
					if((ch & 0xf8) == 0xf0)
					{
						// 4 bytes
						*p1 = (wchar_t(ch & 0x07) << 18 ) | ((wchar_t(p[1] & 0x3f) << 12 )) | ((wchar_t(p[2] & 0x3f) << 6 )) | ((wchar_t(p[3] & 0x3f) << 0 ));
						p+= 4;
					}
					else  if((ch & 0xf0) == 0xe0)
					{
						// 3 bytes
						*p1 = (wchar_t(ch & 0x0f) << 12 ) | ((wchar_t(p[1] & 0x3f) << 6 )) | ((wchar_t(p[2] & 0x3f) << 0 ));
						p+= 3;
					}
					else if((ch & 0xe0) == 0xc0)
					{
						// 2 bytes
						*p1 = (wchar_t(ch & 0x1f) << 6 ) | ((wchar_t(p[1] & 0x3f) << 0 ));
						p+= 2;
					}
					else if((ch & 0x80) == 0)
					{
						// 1 byte
						*p1 = ch & 0x7f;
						p+= 1;
					}
					else
					{
						E_ASSERT(0);
					}
					p1++;
				}
				*p1 = 0;
			}
			return;
		default:
			E_TRACE(L"[nb] (WW) Unsupported charset");
			break;
		}
FALLBACK:
		{
			buf_size = strlen(_r) + 1;
			buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));

			wchar_t * p = buf;
			do
			{
				*p++=*_r;
			}while(*_r++);

#ifdef NB_CFG_VERBOSE
			E_TRACE_LINE(L"? \"" + *this + L"\"");
#endif
		}
	}

	stringw::stringw(const char * _r, Charset _srcCharset)
	{
		init(_r, _srcCharset);
	}

	stringw::stringw(const stringa & _r, Charset _srcCharset)
	{
		init(_r.c_str(), _srcCharset);
	}

	stringw::stringw(const wchar_t * _w_str_init)
	{
		if(_w_str_init == 0)
		{
			buf_size = 1;
			buf = (wchar_t*) malloc(1 * sizeof(wchar_t));
			buf[0] = 0;
			return;
		}
		const wchar_t * _r = _w_str_init;
		buf_size = wcslen(_r) + 1;
		buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
		wcscpy_s(buf, buf_size, _r);
	}

	stringw::stringw(const stringw & _r)
	{
		buf_size = wcslen(_r.buf) + 1;
		buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
		wcscpy_s(buf, buf_size, _r.buf);
	}

	const stringw & stringw::operator=(const stringw & _r)
	{
		if(this != &_r)
		{
			reserve(wcslen(_r.buf) + 1);
			wcscpy_s(buf, buf_size, _r.buf);
		}
		return *this;
	}

	static inline void InitTextWFromBuf(wchar_t * & _buf, int & _size, const char * _src)
	{
		_size = strlen(_src) + 1;
		_buf = (wchar_t*) malloc(_size * sizeof(wchar_t));
		//wcscpy_s(buf, buf_size, buf1);
		const char * p = _src;
		wchar_t * p1 = _buf;
		do
		{
			*p1++ = *p++;
		}while(*p);
		*p1=0;
	}

	stringw::stringw(int _r)
	{
		char buf1[128];
		sprintf(buf1, "%d", _r);
		InitTextWFromBuf(buf, buf_size, buf1);
	}

	stringw::stringw(uint _r)
	{
		char buf1[128];
		sprintf(buf1, "%u", _r);
		InitTextWFromBuf(buf, buf_size, buf1);
	}

	stringw::stringw(double _r, int _digits)
	{
		char fmt[32];
		sprintf(fmt, "%%.%dlf", _digits);
		char buf1[128];
		sprintf(buf1,fmt, _r);
		InitTextWFromBuf(buf, buf_size, buf1);
	}


	stringw::stringw(float _r, int _digits)
	{
		char fmt[32];
		sprintf(fmt, "%%.%df", _digits);
		char buf1[128];
		sprintf(buf1, fmt, _r);
		InitTextWFromBuf(buf, buf_size, buf1);
	}

	stringw::stringw(wchar_t _r)
	{
		buf_size = 2;
		buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
		buf[0] = _r;
		buf[1] = 0;
	}

	stringw::stringw(bool _r)
	{
		buf_size = 6;
		buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
		wcscpy_s(buf, buf_size, _r ? L"true" : L"false");
	}

	//stringw::stringw(unsigned char _r)
	//{
	//	buf_size = 3;
	//	buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
	//	int i;
	//	i = (_r >> 4) & 0x0f;
	//	buf[0] = i > 9 ? 'A' + i - 10 : '0' + i;
	//	i= _r & 0x0f;
	//	buf[1] = i > 9 ? 'A' + i - 10 : '0' + i;
	//	buf[2] = 0;
	//}

	stringw::~stringw()
	{
		free(buf);
	}

	wchar_t * stringw::c_str() const
	{
		return buf;
	}

	int stringw::length() const
	{
		return wcslen(buf);
	}

	void stringw::reserve(int _capacity)
	{
		if(_capacity < length())
		{
			_capacity = length();
		}

		if(buf_size - 1 < _capacity)
		{
			buf_size = _capacity + 1;
			buf = (wchar_t*) realloc(buf, buf_size * sizeof(wchar_t));
		}
	}

	//int stringw::Capacity() const
	//{
	//	return buf_size - 1;
	//}

/*	stringw::operator const wchar_t*() const
	{
		return buf;
	}

	stringw::operator wchar_t*()
	{
		return buf;
	}
*/
	wchar_t stringw::operator[](int _index_w) const
	{
	    E_ASSERT(_index_w < buf_size);
		return buf[_index_w];
	}

	wchar_t & stringw::operator[](int _index_w)
	{
        E_ASSERT(_index_w < buf_size);
		return buf[_index_w];
	}

	int stringw::compare(const stringw & _r) const
	{
		return wcscmp(buf, _r.buf);
	}

	int stringw::icompare(const stringw & _r) const
	{
//#if defined(__CYGWIN__)
//#	error
//#elif defined(NB_LINUX)
//		return wcscasecmp(buf, _r.buf);
//#else
//		return _wcsicmp(buf, _r.buf);
//#endif
		return e::icompare<wchar_t>(buf, _r.buf);
	}

	bool stringw::operator==(const stringw & _r) const
	{
		return compare(_r) == 0;
	}

	bool stringw::operator!=(const stringw & _r) const
	{
		return compare(_r) != 0;
	}

	bool stringw::operator<(const stringw & _r) const
	{
		return compare(_r) < 0;
	}

	//bool stringw::operator<=(const stringw & _r) const
	//{
	//	return compare(_r) <= 0;
	//}

	bool stringw::operator>(const stringw & _r) const
	{
		return compare(_r) > 0;
	}

	//bool stringw::operator>=(const stringw & _r) const
	//{
	//	return compare(_r) >= 0;
	//}

	int stringw::to_int() const
	{
	    //wchar_t * p;
	   // return wcstol(buf, &p, 0);
		int ret = 0;
		bool negative;
		const wchar_t * p = buf;
		while(*p && !iswdigit(*p) && *p != '-')
		{
			p++;
		}

		if(*p == '-')
		{
			negative = true;
			p++;
		}
		else
		{
			negative = false;
		}

		while(*p >= '0' && *p <= '9')
		{
			ret*= 10;
			ret+= *p++ - '0';
		}
		if(negative)
		{
			ret = -ret;
		}
		return ret;
	}

	float stringw::to_float() const
	{
		wchar_t * p;
		return (float)wcstod(buf, &p);
		//float ret = 0;
		//bool negative;
		//const wchar_t * p = buf;
		//while(*p && !iswdigit(*p) && *p != '-' && *p != '.')
		//{
		//	p++;
		//}

		//if(*p == '-')
		//{
		//	negative = true;
		//	p++;
		//}
		//else
		//{
		//	negative = false;
		//}

		//while(*p >= '0' && *p <= '9')
		//{
		//	ret*= 10;
		//	ret+= *p++ - '0';
		//}

		//if(*p == '.')
		//{
		//	p++;
		//	const float t[] = { 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f};
		//	uint n = 0;
		//	while(n < sizeof(t) / sizeof(float) && *p >= '0' && *p <= '9')
		//	{
		//		ret+= t[n] * float(*p++ - '0');
		//		n++;
		//	}
		//}

		//if(negative)
		//{
		//	ret = -ret;
		//}
		//return ret;
	}

	bool stringw::to_bool() const
	{
		if(length() == 0)
		{
			return false;
		}

		wchar_t ch = buf[0];
		return ch == L'T' || ch == L't' || ch == L'1' /*|| icompare(_TT("true")) == 0*/;
	}

	double stringw::to_double() const
	{
	    wchar_t * p;
		return wcstod(buf, &p);
	}

	stringw stringw::operator+(const stringw & _r) const
	{
		//stringw * pThis = const_cast<stringw *>(this);
		stringw s(*this);
		s.reserve(s.length() + _r.length());
		wcscat_s(s.buf, s.buf_size, _r.buf);
		return s;
	}

	void stringw::append(const stringw & _r)
	{
		reserve(length() + _r.length());
		wcscat_s(buf, buf_size, _r.buf);
	}

	const stringw & stringw::operator+=(const stringw & _r)
	{
		reserve(length() + _r.length());
		wcscat_s(buf, buf_size, _r.buf);
		return *this;
	}

	/*int stringw::Find(wchar_t _ch) const
	{
		wchar_t * p = wcschr(buf, _ch);
		if(p != NULL)
		{
			return (int)(p - buf);
		}
		else
		{
			return -1;
		}
	}

	int stringw::Find(const wchar_t * _chs) const
	{
		wchar_t * p = NULL;
		while(*_chs)
		{
			wchar_t * p1 = wcschr(buf, *_chs);
			p = p1 == NULL ? p : (p > p1 ? p : p1);
			_chs++;
		}

		if(p != NULL)
		{
			return (int)(p - buf);
		}
		else
		{
			return -1;
		}
	}*/

	int stringw::replace(const wchar_t * _from, const wchar_t * _to)
	{
		int lenFom = wcslen(_from);
		int lenTo  = wcslen(_to);
		int delta  = lenTo - lenFom;
		int count  = 0;
		const wchar_t * p;
		for(p = buf; *p;)
		{
			if(memcmp(p, _from, lenFom * sizeof(wchar_t)) == 0)
			{
				count ++;
				p+= lenFom;
			}
			else
			{
				p++;
			}
		}

		size_t newSize = buf_size + delta * count;
		wchar_t * newBuf = (wchar_t *) malloc(sizeof(wchar_t) * newSize);
		memset(newBuf, 0, sizeof(wchar_t) * newSize);
		wchar_t *p1 = newBuf;
		for(p = buf; *p;)
		{
			if(memcmp(p, _from, lenFom * sizeof(wchar_t)) == 0)
			{
				memcpy(p1, _to, lenTo * sizeof(wchar_t));
				p1+=lenTo;
				p+= lenFom;
			}
			else
			{
				*p1=*p;
				p++;
				p1++;
			}
		}

		free(buf);
		buf = newBuf;
		buf_size = newSize;

		return count;
	}
	int stringw::replace(wchar_t _from, wchar_t _to)
	{
		int ret = 0;
		wchar_t * p = buf;
		while(*p)
		{
			if(*p == _from)
			{
				*p = _to;
				ret++;
			}
			*p++;
		}
		return ret;
	}

	stringw stringw::substr(int _from, int _len) const
	{
		int totalLen = (int)length();
		if(_from >= totalLen)
		{
			return L"";
		}
		if(_len == -1 || _from + _len > totalLen)
		{
			_len = totalLen - _from;
		}

		stringw ret;
		ret.reserve(_len);
		memcpy(ret.buf, buf + _from, _len * sizeof(wchar_t));
		ret.buf[_len] = 0;
		return ret;
	}

	//void stringw::Delete(int _from, int _len)
	//{
	//	int totalLen = (int)length();
	//	if(_from >= totalLen)
	//	{
	//		return;
	//	}
	//	if(_len == -1 || _from + _len > totalLen)
	//	{
	//		_len = totalLen - _from;
	//	}
	//	*this = substr(0, _from) + substr(_from + _len, -1);
	//}

	int stringw::find(wchar_t _ch, int _pos) const
	{
		E_ASSERT(_pos >= 0);
		if(_pos >= 0 && _pos < length())
		{
			wchar_t * p = wcschr(buf + _pos, _ch);
			if(p != NULL)
			{
				return (int)(p - buf);
			}
		}
		return -1;
	}

	int stringw::find_str(const wchar_t * _s, int _pos) const
	{
		E_ASSERT(_pos >= 0);
		if(_pos >= 0 && _pos < length())
		{
			const wchar_t * p = buf + _pos;
			const wchar_t * p1 = wcsstr(p, _s);
			if(p1)
			{
				return _pos + (p1 - p);
			}
		}

		return -1;
	}

	int stringw::find_any(const wchar_t * _chs, int _pos) const
	{
		E_ASSERT(_pos >= 0);
		if(_pos < length())
		{
			wchar_t * p = NULL;
			while(*_chs)
			{
				wchar_t * p1 = wcschr(buf + _pos, *_chs);
				p = p1 == NULL ? p : (p < p1 && p != NULL ? p : p1);
				_chs++;
			}
			if(p != NULL)
			{
				return (int)(p - buf);
			}
		}
		return -1;
	}

	int stringw::rfind(wchar_t _ch, int _pos) const
	{
		E_ASSERT(_pos >= 0 || _pos == -1);
		if(_pos == -1)
		{
			_pos = length();
		}

		for(;_pos >=0; _pos--)
		{
			if(buf[_pos] == _ch)
			{
				return _pos;
			}
		}
		return -1;
	}

	int stringw::rfind_any(const wchar_t * _chs, int _pos) const
	{
		E_ASSERT(_pos >= 0 || _pos == -1);
		if(_pos == -1)
		{
			_pos = length();
		}

		for(;_pos >=0; _pos--)
		{
			const wchar_t * p = _chs;
			while(*p)
			{
				if(buf[_pos] == *p++)
				{
					return _pos;
				}
			}
		}
		return -1;
	}

	bool stringw::empty() const
	{
		return length() == 0;
	}

	//stringw stringw::operator+(const wchar_t * _r) const
	//{
	//	return operator+(stringw(_r));
	//}
	void stringw::clear()
	{
		free(buf);
		buf_size = 1;
		buf = (wchar_t*) malloc(sizeof(wchar_t));
		buf[0] = 0;
	}

	void stringw::append(wchar_t _ch)
	{
		int len = length() ;
		reserve(len + 1);
		buf[len++] = _ch;
		buf[len] = 0;
	}

	void stringw::insert(int _index, const stringw & _r)
	{
		if(_r.empty())
		{
			return;
		}
		int len = length();
		E_BASIC_ASSERT(_index >= 0 && _index <= len);
		int len1 = _r.length();
		reserve(len + len1);
		wchar_t * p0 = buf + len;
		wchar_t * pe = buf + _index;
		wchar_t * p1 = buf + len + len1;
		while(p0 >= pe)
		{
			*p1-- = *p0--;
		}
		memcpy(pe, _r.buf, len1 * sizeof(wchar_t));
	}

	void stringw::insert(int _index, wchar_t _ch)
	{
		int len = length();
		E_BASIC_ASSERT(_index >= 0 && _index <= len);
		reserve(len + 1);
		for(int i = len + 1; i > _index; i--)
		{
			buf[i] = buf[i-1];
		}
		buf[_index] = _ch;
	}

	//int stringw::HexDecode(void * _bufOut, int _size) const
	//{
	//	E_ASSERT(_size >= 0);
	//	unsigned char * _out = (unsigned char*)_bufOut;
	//	int len = length();
	//	int n = 0;
	//	for(int i=0; i<_size; i++)
	//	{
	//		int j = i + i;
	//		if(j+1 < len)
	//		{
	//			unsigned char b0, b1;
	//			wchar_t ch = buf[j];
	//			if(ch >='0' && ch <='9')
	//			{
	//				b0 = ch - '0';
	//			}
	//			else if(ch >='A' && ch <='F')
	//			{
	//				b0 = ch - 'A' + 10;
	//			}
	//			else if(ch >='a' && ch <='f')
	//			{
	//				b0 = ch - 'a' + 10;
	//			}
	//			else
	//			{
	//				_out[i] = 0;
	//				n++;
	//				continue;
	//			}
	//			ch = buf[j+1];
	//			if(ch >='0' && ch <='9')
	//			{
	//				b1 = ch - '0';
	//			}
	//			else if(ch >='A' && ch <='F')
	//			{
	//				b1 = ch - 'A' + 10;
	//			}
	//			else if(ch >='a' && ch <='f')
	//			{
	//				b1 = ch - 'a' + 10;
	//			}
	//			else
	//			{
	//				_out[i] = 0;
	//				n++;
	//				continue;
	//			}

	//			_out[i] = (b0  << 4) | b1;
	//			n++;
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}

	//	return n;
	//}

	//void stringw::HexEncode(const void * _bufIn, int _size)
	//{
	//	E_ASSERT(_size >= 0);
	//	unsigned char * _in = (unsigned char*)_bufIn;
	//	reserve(_size * 2);
	//	for(int i=0; i<_size; i++)
	//	{
	//		int j = i + i;
	//		unsigned char b0, b1;
	//		b0 = (_in[i] >> 4) & 0x0f;
	//		b1 = _in[i] & 0x0f;
	//		if(b0 > 9)
	//		{
	//			buf[j] = b0 - 10 + 'A';
	//		}
	//		else
	//		{
	//			buf[j] = b0 + '0';
	//		}

	//		if(b1 > 9)
	//		{
	//			buf[j+1] = b1 - 10 + 'A';
	//		}
	//		else
	//		{
	//			buf[j+1] = b1 + '0';
	//		}

	//	}
	//	buf[_size * 2] = 0;
	//}

	stringw stringw::format(const wchar_t * _format, va_list _argptr)
	{
		stringw s;
#ifdef NB_WINDOWS
		int bufSize = _vscwprintf(_format, _argptr) + 1;
		s.reserve(bufSize);
#   ifdef __GNUC__
		_vsnwprintf(s.c_str(), bufSize, _format, _argptr);
#   else
		vswprintf_s(s.c_str(), bufSize, _format, _argptr);
#   endif
#else
		int bufSize = 3600; // TODO: ...
		s.reserve(bufSize);
		vswprintf (s.c_str(), bufSize, _format, _argptr);
#endif
		return s;
	}

	stringw stringw::format(const wchar_t * _format, ...)
	{
		va_list argptr;
		va_start(argptr, _format);
		stringw s = format(_format, argptr);
		va_end(argptr);
		return s;
	}

	void stringw::trim()
	{
		wchar_t * a = buf;
		wchar_t * b = buf + wcslen(buf) - 1;
		while(*a && iswspace(*a)) a++;
		while(b >= a && iswspace(*b)) b--;
		int len = b - a + 1;
		if(len > 0)
		{
			memmove(buf, a, len * sizeof(wchar_t));
			buf[len] = 0;
		}
		else
		{
			buf[0] = 0;
		}
	}

//	void stringw::TrimStart()
//	{
//		wchar_t * a = buf;
//		wchar_t * b = buf + wcslen(buf) - 1;
//		while(*a && iswspace(*a)) a++;
//		//while(b >= a && iswspace(*b)) b--;
//		int len = b - a + 1;
//		if(len > 0)
//		{
//			memmove(buf, a, len * sizeof(wchar_t));
//			buf[len] = 0;
//		}
//		else
//		{
//			buf[0] = 0;
//		}
//	}
//
//	void stringw::TrimEnd()
//	{
//		wchar_t * a = buf;
//		wchar_t * b = buf + wcslen(buf) - 1;
//		//while(*a && iswspace(*a)) a++;
//		while(b >= a && iswspace(*b)) b--;
//		int len = b - a + 1;
//		if(len > 0)
//		{
//			memmove(buf, a, len * sizeof(wchar_t));
//			buf[len] = 0;
//		}
//		else
//		{
//			buf[0] = 0;
//		}
//	}

//	void stringw::trim(const wchar_t * _chs)
//	{
//		wchar_t * a = buf;
//		wchar_t * b = buf + wcslen(buf) - 1;
//		while(*a && wcschr(_chs, *a)) a++;
//		while(b >= a && wcschr(_chs, *b)) b--;
//		int len = b - a + 1;
//		if(len > 0)
//		{
//			memmove(buf, a, len * sizeof(wchar_t));
//			buf[len] = 0;
//		}
//		else
//		{
//			buf[0] = 0;
//		}
//	}

	//void stringw::TrimStart(const wchar_t * _chs)
	//{
	//	wchar_t * a = buf;
	//	wchar_t * b = buf + wcslen(buf) - 1;
	//	while(*a && wcschr(_chs, *a)) a++;
	//	//while(b >= a && wcschr(_chs, *b)) b--;
	//	int len = b - a + 1;
	//	if(len > 0)
	//	{
	//		memmove(buf, a, len * sizeof(wchar_t));
	//		buf[len] = 0;
	//	}
	//	else
	//	{
	//		buf[0] = 0;
	//	}
	//}

	//void stringw::TrimEnd(const wchar_t * _chs)
	//{
	//	wchar_t * a = buf;
	//	wchar_t * b = buf + wcslen(buf) - 1;
	//	//while(*a && wcschr(_chs, *a)) a++;
	//	while(b >= a && wcschr(_chs, *b)) b--;
	//	int len = b - a + 1;
	//	if(len > 0)
	//	{
	//		memmove(buf, a, len * sizeof(wchar_t));
	//		buf[len] = 0;
	//	}
	//	else
	//	{
	//		buf[0] = 0;
	//	}
	//}

	void stringw::to_upper()
	{
		wchar_t * p = buf;
		while(*p)
		{
			*p = toupper(*p);
			p++;
		}
	}

	void stringw::to_lower()
	{
		wchar_t * p = buf;
		while(*p)
		{
			*p = tolower(*p);
			p++;
		}
	}

	//stringw stringw::ExtractFirstWord(const wchar_t * _chs, bool _erase)
	//{
	//	stringw ret;
	//	int n = find(_chs);
	//	if(n == -1)
	//	{
	//		ret = *this;
	//		this->clear();
	//	}
	//	else
	//	{
	//		ret = substr(0, n);
	//		if(_erase)
	//		{
	//			*this = substr(n+1, -1);
	//		}
	//	}
	//	return ret;
	//}

	string BytesToMKBText(uint _bytes) //BUG: "1000" may display as wrong value
	{
		if(_bytes >= 900000)
		{
			float f = float(_bytes) / float(1024 * 1024) + 0.05f;
			int a = (int)f;
			int b = (int)(f*10) % 10;
			return string::format(L"%4d.%1dM", a, b);
		}
		else if(_bytes >= 900)
		{
			float f = float(_bytes) / float(1024) + 0.05f;
			int a = (int)f;
			int b = (int)(f*10) % 10;
			return string::format(L"%4d.%1dK", a, b);
		}
		else
		{
			return string::format(L"%6d", int(_bytes));
		}
	}


	string ByteToHex(uint8 _byte)
	{
		//buf_size = 3;
		//buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
		wchar_t buf[3];
		int i;
		i = (_byte >> 4) & 0x0f;
		buf[0] = i > 9 ? 'A' + i - 10 : '0' + i;
		i= _byte & 0x0f;
		buf[1] = i > 9 ? 'A' + i - 10 : '0' + i;
		buf[2] = 0;
		return buf;
	}

	string PointerToHex(void * _p)
	{
		const int n = sizeof(_p) * 2;
		wchar_t buf[n+1];
		int j = sizeof(_p) - 1;
		int m = 0;
		for(; m < n; j--)
		{
			uint8 b = ((uint8*)&_p)[j];
			int i;
			i = (b >> 4) & 0x0f;
			buf[m++] = i > 9 ? 'A' + i - 10 : '0' + i;
			i= b & 0x0f;
			buf[m++] = i > 9 ? 'A' + i - 10 : '0' + i;
		}
		buf[m] = 0;
		return buf;
	}

		//stringa aa(0);
	//stringw bb(0);

	//stringw a(0);
	//stringa b(0);
	//int a1 = aa.to_int();
	//int b1 = bb.to_int();
	//int c1 = a.to_int();
	//int d1 = b.to_int();
	//float a2 = aa.to_float();
	//float b2 = bb.to_float();
	//float c2 = a.to_float();
	//float d2 = b.to_float();

	stringw QuoteString(stringw _s)
	{
		_s.replace(L"\"", L"\\\"");
		return L"\"" + _s + L"\"";
	}

	stringa QuoteString(stringa _s)
	{
		_s.replace("\"", "\\\"");
		return "\"" + _s + "\"";
	}

	stringw QuoteString(const wchar_t * _s)
	{
		return QuoteString(stringw(_s));
	}

	stringa QuoteString(const char * _s)
	{
		return QuoteString(stringa(_s));
	}

#ifdef E_CFG_UNIT_TEST

	E_UNIT_TEST_CASE(IntegerTypeSize)
	{
		E_UNIT_TEST_ASSERT(sizeof(int8) == 1);
		E_UNIT_TEST_ASSERT(sizeof(uint8) == 1);
		E_UNIT_TEST_ASSERT(sizeof(int16) == 2);
		E_UNIT_TEST_ASSERT(sizeof(uint16) == 2);
		E_UNIT_TEST_ASSERT(sizeof(int32) == 4);
		E_UNIT_TEST_ASSERT(sizeof(uint32) == 4);
		E_UNIT_TEST_ASSERT(sizeof(int64) == 8);
		E_UNIT_TEST_ASSERT(sizeof(uint64) == 8);
		//E_UNIT_TEST_ASSERT(sizeof(Uint_PtrSize) == sizeof(int*));
		return true;
	}

	E_UNIT_TEST_CASE(stringa)
	{
		// int to text
		E_UNIT_TEST_ASSERT(stringa(0) == stringa("0"));
		E_UNIT_TEST_ASSERT(stringa(-0) == stringa("0"));
		E_UNIT_TEST_ASSERT(stringa(3) == stringa("3"));
		E_UNIT_TEST_ASSERT(stringa(-3) == stringa("-3"));
		E_UNIT_TEST_ASSERT(stringa(1234) == stringa("1234"));
		E_UNIT_TEST_ASSERT(stringa(-1234) == stringa("-1234"));
		E_UNIT_TEST_ASSERT(stringa(5000) == stringa("5000"));
		E_UNIT_TEST_ASSERT(stringa(-5000) == stringa("-5000"));

		// uint to text
		E_UNIT_TEST_ASSERT(stringa((uint)0) == stringa("0"));
		E_UNIT_TEST_ASSERT(stringa((uint)0xffffffff) == stringa("4294967295"));
		E_UNIT_TEST_ASSERT(stringa((uint)3) == stringa("3"));
		E_UNIT_TEST_ASSERT(stringa((uint)1234) == stringa("1234"));
		E_UNIT_TEST_ASSERT(stringa((uint)5000) == stringa("5000"));


		// text to int
		E_UNIT_TEST_ASSERT(stringa("0").to_int() == 0);
		E_UNIT_TEST_ASSERT(stringa("-0").to_int() == 0);
		E_UNIT_TEST_ASSERT(stringa("3").to_int() == 3);
		E_UNIT_TEST_ASSERT(stringa("-3").to_int() == -3);
		E_UNIT_TEST_ASSERT(stringa("1234").to_int() == 1234);
		E_UNIT_TEST_ASSERT(stringa("-1234").to_int() == -1234);
		E_UNIT_TEST_ASSERT(stringa("5000").to_int() == 5000);
		E_UNIT_TEST_ASSERT(stringa("-5000").to_int() == -5000);

		// float to text
		//E_UNIT_TEST_ASSERT(stringa(0.0f) == stringa("0"));
		//E_UNIT_TEST_ASSERT(stringa(-0.0f) == stringa("0"));
		//E_UNIT_TEST_ASSERT(stringa(0.12345f) == stringa("0.12345"));
		//E_UNIT_TEST_ASSERT(stringa(-0.12345f) == stringa("-0.12345"));
		//E_UNIT_TEST_ASSERT(stringa(0.01f) == stringa("0.01"));
		//E_UNIT_TEST_ASSERT(stringa(-0.01f) == stringa("-0.01"));
		//E_UNIT_TEST_ASSERT(stringa(9.00f) == stringa("9"));
		//E_UNIT_TEST_ASSERT(stringa(-9.00f) == stringa("-9"));

		// text to float
		E_UNIT_TEST_ASSERT(stringa("0").to_float() == 0.0f);
		E_UNIT_TEST_ASSERT(stringa("-0").to_float() == -0.0f);
		E_UNIT_TEST_ASSERT(stringa(".01").to_float() == 0.01f);
		E_UNIT_TEST_ASSERT(stringa("-.01").to_float() == -0.01f);
		E_UNIT_TEST_ASSERT(stringa(".").to_float() == 0.0f);
		E_UNIT_TEST_ASSERT(stringa("-.").to_float() == -0.0f);
		E_UNIT_TEST_ASSERT(stringa("qwert").to_float() == 0.0f);
		E_UNIT_TEST_ASSERT(stringa("0.12345").to_float() == 0.12345f);
		E_UNIT_TEST_ASSERT(stringa("-0.12345").to_float() == -0.12345f);
		E_UNIT_TEST_ASSERT(stringa("0.01").to_float() == 0.01f);
		E_UNIT_TEST_ASSERT(stringa("-0.01").to_float() == -0.01f);
		E_UNIT_TEST_ASSERT(stringa("9").to_float() == 9.0f);
		E_UNIT_TEST_ASSERT(stringa("-9").to_float() == -9.0f);
		E_UNIT_TEST_ASSERT(stringa("56.f").to_float() == 56.0f);
		E_UNIT_TEST_ASSERT(stringa("-56.f").to_float() == -56.0f);

		// case insensive compare
		E_UNIT_TEST_ASSERT(stringa("AqA").icompare(stringa("aqa")) == 0);
		E_UNIT_TEST_ASSERT(stringa("AqAa").icompare(stringa("aqa")) > 0);
		E_UNIT_TEST_ASSERT(stringa("AqAa").icompare(stringa("aqaaa")) < 0);

		// case sensive compare
		E_UNIT_TEST_ASSERT(stringa("AqA").compare(stringa("aqa")) < 0);
		return true;
	}

	E_UNIT_TEST_CASE(stringw)
	{
		// int to text
		E_UNIT_TEST_ASSERT(stringw(0) == stringw("0"));
		E_UNIT_TEST_ASSERT(stringw(-0) == stringw("0"));
		E_UNIT_TEST_ASSERT(stringw(3) == stringw("3"));
		E_UNIT_TEST_ASSERT(stringw(-3) == stringw("-3"));
		E_UNIT_TEST_ASSERT(stringw(1234) == stringw("1234"));
		E_UNIT_TEST_ASSERT(stringw(-1234) == stringw("-1234"));
		E_UNIT_TEST_ASSERT(stringw(5000) == stringw("5000"));
		E_UNIT_TEST_ASSERT(stringw(-5000) == stringw("-5000"));

		// uint to text
		E_UNIT_TEST_ASSERT(stringw((uint)0) == stringw("0"));
		E_UNIT_TEST_ASSERT(stringw((uint)0xffffffff) == stringw("4294967295"));
		E_UNIT_TEST_ASSERT(stringw((uint)3) == stringw("3"));
		E_UNIT_TEST_ASSERT(stringw((uint)1234) == stringw("1234"));
		E_UNIT_TEST_ASSERT(stringw((uint)5000) == stringw("5000"));

		// text to int
		E_UNIT_TEST_ASSERT(stringw("0").to_int() == 0);
		E_UNIT_TEST_ASSERT(stringw("-0").to_int() == 0);
		E_UNIT_TEST_ASSERT(stringw("3").to_int() == 3);
		E_UNIT_TEST_ASSERT(stringw("-3").to_int() == -3);
		E_UNIT_TEST_ASSERT(stringw("1234").to_int() == 1234);
		E_UNIT_TEST_ASSERT(stringw("-1234").to_int() == -1234);
		E_UNIT_TEST_ASSERT(stringw("5000").to_int() == 5000);
		E_UNIT_TEST_ASSERT(stringw("-5000").to_int() == -5000);

		// float to text
		//E_UNIT_TEST_ASSERT(stringw(0.0f) == stringw("0"));
		//E_UNIT_TEST_ASSERT(stringw(-0.0f) == stringw("0"));
		//E_UNIT_TEST_ASSERT(stringw(0.12345f) == stringw("0.12345"));
		//E_UNIT_TEST_ASSERT(stringw(-0.12345f) == stringw("-0.12345"));
		//E_UNIT_TEST_ASSERT(stringw(0.01f) == stringw("0.01"));
		//E_UNIT_TEST_ASSERT(stringw(-0.01f) == stringw("-0.01"));
		//E_UNIT_TEST_ASSERT(stringw(9.00f) == stringw("9"));
		//E_UNIT_TEST_ASSERT(stringw(-9.00f) == stringw("-9"));

		// text to float
		E_UNIT_TEST_ASSERT(stringw("0").to_float() == 0.0f);
		E_UNIT_TEST_ASSERT(stringw("-0").to_float() == -0.0f);
		E_UNIT_TEST_ASSERT(stringw(".01").to_float() == 0.01f);
		E_UNIT_TEST_ASSERT(stringw("-.01").to_float() == -0.01f);
		E_UNIT_TEST_ASSERT(stringw(".").to_float() == 0.0f);
		E_UNIT_TEST_ASSERT(stringw("-.").to_float() == -0.0f);
		E_UNIT_TEST_ASSERT(stringw("qwert").to_float() == 0.0f);
		E_UNIT_TEST_ASSERT(stringw("0.12345").to_float() == 0.12345f);
		E_UNIT_TEST_ASSERT(stringw("-0.12345").to_float() == -0.12345f);
		E_UNIT_TEST_ASSERT(stringw("0.01").to_float() == 0.01f);
		E_UNIT_TEST_ASSERT(stringw("-0.01").to_float() == -0.01f);
		E_UNIT_TEST_ASSERT(stringw("9").to_float() == 9.0f);
		E_UNIT_TEST_ASSERT(stringw("-9").to_float() == -9.0f);
		E_UNIT_TEST_ASSERT(stringw("56.f").to_float() == 56.0f);
		E_UNIT_TEST_ASSERT(stringw("-56.f").to_float() == -56.0f);

		// case insensive compare
		E_UNIT_TEST_ASSERT(stringw("AqA").icompare(stringw("aqa")) == 0);
		E_UNIT_TEST_ASSERT(stringw("AqAa").icompare(stringw("aqa")) > 0);
		E_UNIT_TEST_ASSERT(stringw("AqAa").icompare(stringw("aqaaa")) < 0);

		// case sensive compare
		E_UNIT_TEST_ASSERT(stringw("AqA").compare(stringw("aqa")) < 0);
		return true;
	}

	E_UNIT_TEST_CASE(Utf8Convert)
	{
		stringw wchar_src = L"\x0074\x0065\x0073\x0074\x5B57\x7B26\x4E32\x0041\x0042\x0043\x4E2D\x6587";
		stringa utf8_src  = "\x74\x65\x73\x74\xE5\xAD\x97\xE7\xAC\xA6\xE4\xB8\xB2\x41\x42\x43\xE4\xB8\xAD\xE6\x96\x87";
		stringa utf8_dst  = stringa(wchar_src, CHARSET_UTF8);
		stringw wchar_dst = stringw(utf8_src, CHARSET_UTF8);
		E_UNIT_TEST_ASSERT(utf8_src == utf8_dst);
		E_UNIT_TEST_ASSERT(wchar_src == wchar_dst);
		return true;
	}
#endif
}
