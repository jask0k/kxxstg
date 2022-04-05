
#ifndef NB_BASIC_DEFS_H
#define NB_BASIC_DEFS_H

namespace e
{
/*
#if !defined(NB_WINDOWS) && !defined(NB_LINUX)
#	if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#		define NB_WINDOWS 1
#	endif

#	if defined(__linux) || defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#		define NB_LINUX 1
#	endif
#endif
*/

    typedef char int8;
    typedef short int16;
	typedef unsigned int uint;
    typedef unsigned char uint8;
    typedef unsigned short uint16;

	// fixed length integers
#if defined(_WIN64) || defined(__x86_64__)
#	define E_64
    typedef int int32;
    typedef unsigned int uint32;
    typedef long long int64;
    typedef unsigned long long uint64;
    typedef long long intx;
	typedef unsigned long long uintx;
#	else
#	define E_32
    typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
    typedef unsigned long long uint64;
    typedef int intx;
	typedef unsigned int uintx;
#	endif

	// char 
//#ifdef UNICODE
	typedef wchar_t Char;
//#   define _T(s) L##s
//#else
//	typedef char Char;
//#   define _T(s) s
//#endif

	// other types

	typedef float real;

	// other definitions
#ifdef __GNUC__
#	define override
#endif

#define E_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define E_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define E_ABS(a) (((a) < 0 ) ? -(a) : (a))

#define E_SAFE_RELEASE(x) {if(x){(x)->Release(); (x)=0;}}
#define E_SAFE_DELETE(x)  {if(x){delete (x); (x)=0;}}

#define _NB_SHARP_(x) #x
#define _NB_M_S_(x) _NB_SHARP_(x)
#ifdef NB_CFG_SRC_LOC
#	define NB_SRC_LOC  __FILE__ "(" _NB_M_S_(__LINE__) "): "
#else
#	define NB_SRC_LOC ""
#endif

}

#endif
