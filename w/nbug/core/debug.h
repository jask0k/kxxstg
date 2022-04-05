#ifndef NB_DEBUG_H
#define NB_DEBUG_H

#include <stdlib.h>
#include <stdarg.h>

#ifdef NB_CFG_MEMCHECK
#	include <new>
#endif

#include <nbug/core/str.h>
#include <nbug/core/debug_.h>

namespace e
{
	bool is_debugger_present();
	void write_log(const string & _message);
	void message(const string & _msg);
	void error(const string & _msg);

	void throwf(const char * _fmt, ...) throw(const char *);
	void throw_posix(const char * _prefix) throw(const char *);
	void throw_winapi(const char * _prefix) throw(const char *);
	void throws(const stringa & _s) throw(const char *);

#ifdef NB_DEBUG
	void assert_failed(const string &, const string &, int);
	void debug_write(const string &);
	void debug_write_line(const string &);
#	define E_ASSERT(x)	       do{ if(!(x)) {::e::assert_failed("ASSERT FAILED: " #x , __FILE__ , __LINE__); E_DEBUG_BREAK;} } while(0)
#	define E_ASSERT1(x, _msg)  do{ if(!(x)) {::e::assert_failed("ASSERT FAILED: " #x " : \n\t" + e::string(_msg), __FILE__ , __LINE__); E_DEBUG_BREAK;} } while(0)
#	define E_TRACE_LINE        ::e::debug_write_line
#	define E_TRACE             ::e::debug_write
#else // NB_DEBUG
#	define E_ASSERT(x)         ((void)0)
#	define E_ASSERT1(x, _msg)  ((void)0)
#	define E_TRACE_LINE(x)     ((void)0)
#	define E_TRACE(x)          ((void)0)
#endif //NB_DEBUG

}

namespace e
{
	void * nd_malloc(size_t _sz);
	void * nd_realloc(void * _p, size_t _sz);
	void * nd_calloc(size_t _num, size_t _sz);
	void   nd_free(void * _p);
} // namespace e

#ifdef NB_CFG_MEMCHECK
namespace e
{
	enum NB_DEBUG_MEMORY_BLOCK_TYPE
	{
		nbDmbtC,
		nbDmbtNew,
		nbDmbtNewArray,
	};
	void   set_debug_memory_break_at_alloc(int _order);
	void * debug_malloc(size_t _sz, const char * _file, int _line, NB_DEBUG_MEMORY_BLOCK_TYPE _type);
	void * debug_realloc(void * _p, size_t _sz, const char * _file, int _line);
	void * debug_calloc(size_t _num, size_t _sz1, const char * _file, int _line);
	void   debug_free(void * _p, NB_DEBUG_MEMORY_BLOCK_TYPE _type);
	void   debug_chage_loc(void * _p, const char * _file, int _line);
#	define malloc(sz) e::debug_malloc((sz), __FILE__, __LINE__, e::nbDmbtC)
#	define realloc(p, sz) e::debug_realloc((p), (sz), __FILE__, __LINE__)
#	define calloc(num, sz) e::debug_calloc((num), (sz), __FILE__, __LINE__)
#	define free(p) e::debug_free((p), e::nbDmbtC)

} // namespace e

void * operator new(size_t) throw (std::bad_alloc);
void operator delete(void *) throw (std::bad_alloc);
void * operator new[](size_t) throw (std::bad_alloc);
void operator delete[](void *) throw (std::bad_alloc);
void * operator new(size_t, const char *, int);
void operator delete(void *, const char *, int);
void * operator new[](size_t, const char *, int);
void operator delete[](void *, const char *, int);

#	define enew new(__FILE__, __LINE__) // incompitable with STL.
#else // NB_CFG_MEMCHECK
#	define enew new
#endif // NB_CFG_MEMCHECK


#endif // NB_DEBUG_H
