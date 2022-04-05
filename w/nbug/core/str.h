
#ifndef E_STRING_H
#define E_STRING_H

#include <string.h>
#include <stdarg.h>
#include <nbug/core/def.h>

namespace e
{
	enum Charset
	{
		CHARSET_UNKOWN,
		CHARSET_LOCALE = CHARSET_UNKOWN, // LC_CTYPE
		CHARSET_UTF8,
		CHARSET_UTF16LE, // Windows Unicode
		CHARSET_UTF16 = CHARSET_UTF16LE,
		CHARSET_UTF16BE,
		CHARSET_UTF32LE,
		CHARSET_UTF32 = CHARSET_UTF32LE,
		CHARSET_UTF32BE,
#ifdef NB_LINUX
		CHARSET_INTERNAL = CHARSET_UTF32,
#else
		CHARSET_INTERNAL = CHARSET_UTF16,
#endif
	};

	class stringw;
	class stringa
	{
	private:
		char * buf;
		int buf_size;
	public:
		stringa();
		stringa(const char * _r);
		stringa(const stringa & _r);
		stringa(const wchar_t * _r, Charset _dstCharset = CHARSET_LOCALE);
		stringa(const stringw & _r, Charset _dstCharset = CHARSET_LOCALE);
		const stringa & operator=(const stringa & _r);
		~stringa();
		int length() const;
		void reserve(int _capacity); //alloc space, but not fill with 0
		//int Capacity() const;
		//operator char *() const;
		char * c_str() const;
		char operator[](int _index) const;
		char & operator[](int _index);
		int compare(const stringa & _r) const;
		int icompare(const stringa & _r) const;
		bool operator==(const stringa & _r) const;
		bool operator!=(const stringa & _r) const;
		bool operator<(const stringa & _r) const;
		//bool operator<=(const stringa & _r) const;
		bool operator>(const stringa & _r) const;
		//bool operator>=(const stringa & _r) const;
		explicit stringa(char _r); // 'A' => "A"
		explicit stringa(int _r);
		explicit stringa(uint _r);
		explicit stringa(double _r, int _digits=2);
		explicit stringa(float _r, int _digits=2);
		explicit stringa(bool _r);
		//explicit stringa(unsigned char _r); // 'A' => "41"
		int to_int() const;
		float to_float() const;
		double to_double() const;
		bool to_bool() const;
		stringa operator+(const stringa & _r) const;
		void append(const stringa & _r);
		void append(char _ch);
		void insert(int _index, const stringa & _r);
		void insert(int _index, char _ch);
		const stringa & operator+=(const stringa & _r);
		int find(char _ch, int _fromPos = 0) const;
		int find_any(const char * _chs, int _fromPos = 0) const;
		int rfind(char _ch, int _fromPos = -1) const;
		int rfind_any(const char * _chs, int _fromPos = -1) const;
		int find_str(const char * _s, int _fromPos = 0) const;
		int replace(const char * _from, const char * _to);
		int replace(char _from, char _to);
		stringa substr(int _from, int _len) const;
		bool empty() const;
		void clear();
		static stringa format(const char * _format, va_list _argptr);
		static stringa format(const char * _format, ...);
		void trim();
		void to_upper();
		void to_lower();
	private:
		void init(const wchar_t * _r, Charset _dstCharset);
	};

	inline stringa operator+(const char * _l, const stringa & _r)
	{ return stringa(_l) + _r; }

	inline stringa operator+(const wchar_t * _l, const stringa & _r)
	{ return stringa(_l) + _r; }

	class stringw
	{
	private:
		wchar_t * buf;
		int buf_size;
	public:
		stringw();
		stringw(const wchar_t * _r);
		stringw(const stringw & _r);
		stringw(const char * _r, Charset _srcCharset = CHARSET_LOCALE);
		stringw(const stringa & _r, Charset _srcCharset = CHARSET_LOCALE);
		const stringw & operator=(const stringw & _r);
		~stringw();
		int length() const;
		void reserve(int _capacity); //alloc space, but not fill with 0
		wchar_t * c_str() const;
		wchar_t operator[](int _index) const;
		wchar_t & operator[](int _index);
		int compare(const stringw & _r) const;
		int icompare(const stringw & _r) const;
		bool operator==(const stringw & _r) const;
		bool operator!=(const stringw & _r) const;
		bool operator<(const stringw & _r) const;
		bool operator>(const stringw & _r) const;
		explicit stringw(wchar_t _r); // 'A' => "A"
		explicit stringw(int _r);
		explicit stringw(uint _r);
		explicit stringw(double _r, int _digits=2);
		explicit stringw(float _r, int _digits=2);
		explicit stringw(bool _r);
		int to_int() const;
		float to_float() const;
		double to_double() const;
		bool to_bool() const;
		stringw operator+(const stringw & _r) const;
		void append(const stringw & _r);
		void append(wchar_t _ch);
		void insert(int _index, const stringw & _r);
		void insert(int _index, wchar_t _ch);
		const stringw & operator+=(const stringw & _r);
		int find(wchar_t _ch, int _fromPos = 0) const;
		int find_any(const wchar_t * _chs, int _fromPos = 0) const;
		int rfind(wchar_t _ch, int _fromPos = -1) const;
		int rfind_any(const wchar_t * _chs, int _fromPos = -1) const;
		int find_str(const wchar_t * _s, int _fromPos = 0) const;
		int replace(const wchar_t * _from, const wchar_t * _to);
		int replace(wchar_t _from, wchar_t _to);
		stringw substr(int _from, int _len) const;
		bool empty() const;
		void clear();
		static stringw format(const wchar_t * _format, va_list _argptr);
		static stringw format(const wchar_t * _format, ...);
		void trim();
		void to_upper();
		void to_lower();
	private:
		void init(const char * _r, Charset _srcCharset);
	};

	inline stringw operator+(const wchar_t * _l, const stringw & _r)
	{ return stringw(_l) + _r; }

	inline stringw operator+(const char * _l, const stringw & _r)
	{ return stringw(_l) + _r; }


#   define _TT(s) Translate(L##s)
	typedef stringw string;

	string Translate(const string & _text);
	string BytesToMKBText(uint _bytes);
	string ByteToHex(uint8 _byte);
	string PointerToHex(void * _p);
	stringw QuoteString(stringw _s);
	stringa QuoteString(stringa _s);
	stringw QuoteString(const wchar_t * _s);
	stringa QuoteString(const char * _s);
}

#endif

