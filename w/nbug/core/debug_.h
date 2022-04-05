#ifndef E_CORE_BASIC_DEBUG_H
#define E_CORE_BASIC_DEBUG_H

#ifdef NB_DEBUG
#	ifdef _MSC_VER
#		define E_DEBUG_BREAK __debugbreak()
#	else
#		define E_DEBUG_BREAK asm("int $3")
#	endif
#	define E_BASIC_ASSERT(x) do{ if(!(x)) {E_DEBUG_BREAK;} } while(0)
#else
#	define E_DEBUG_BREAK ((void)0)
#	define E_BASIC_ASSERT(x) ((void)0)
#endif

#endif
