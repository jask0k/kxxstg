
#ifndef E_NB_PRIVATE_H
#define E_NB_PRIVATE_H

#ifdef NB_WINDOWS
#   ifndef WINVER
#	    define WINVER 0x0500
#   endif
#	include <windows.h>
#endif

#if !defined(NB_WINDOWS)
#	undef E_CFG_DIRECT3D
#	ifndef E_CFG_OPENGL
#		define E_CFG_OPENGL
#	endif
#endif

namespace e
{
#if defined(E_CFG_DIRECT3D) && defined(E_CFG_OPENGL)
	extern bool g_is_opengl;
#elif defined(E_CFG_DIRECT3D)
	static const bool g_is_opengl = false;
#elif defined(E_CFG_OPENGL)
	static const bool g_is_opengl = true;
#else
#	error !defined(E_CFG_DIRECT3D) &&  !defined(E_CFG_OPENGL)
#endif
}


#ifdef E_CFG_DIRECT3D
#	ifdef NB_DEBUG
#		define D3D_DEBUG_INFO
#	endif
#	include <d3d9.h>
//#	include <d3dx9.h>
#endif

#ifdef E_CFG_OPENGL
#	include <nbug/gl/gl_exts.h>
#endif

#ifdef NB_LINUX
#	include <unistd.h>
//#	include <fcntl.h>
#	include <errno.h>
//#	include <time.h>
#	include <string.h>
#   include <X11/Xatom.h>
#   include <X11/Xlib.h>
#endif

#endif

