
#include "private.h"
#include <nbug/gl/fbo.h>
#include <nbug/gl/tex.h>
#ifdef E_CFG_OPENGL
#	include <nbug/gl/gl_exts.h>
#endif
#include <nbug/core/debug.h>
#include <nbug/gl/graphics.h>
#include <nbug/gl/graphics_.h>

struct IDirect3DSurface9;
namespace e
{
	Fbo::Fbo(GraphicsImp * _g, int _w, int _h)
	{
		//E_TRACE_LINE(L"[nb] Fbo::Fbo()");
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_w > 0 && _h > 0);
		g = _g;
		w = _w;
		h = _h;
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			GLuint n;
			glGenFramebuffersEXT(1, &n);
			//E_ASSERT(glGetError() == 0);
			native = n;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			native = 0;
#endif
		}
		id = g->fboTable.Add(this);
	}

	Fbo::~Fbo()
	{
		// NB_PROFILE_INCLUDE;
		//E_TRACE_LINE(L"[nb] Fbo::~Fbo()");
		if(native)
		{
			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				GLuint n = native;
				glDeleteFramebuffersEXT(1, &n);
				//	E_TRACE_LINE(L"[nb]     delete");
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				int n = DXFBO(native)->Release();
				//E_TRACE_LINE(L"[nb]     Ref=" + string(n));
#endif
			}
			native = 0;
		}

		tex.Detach();
		g->fboTable.Remove(id);
	}

	TexRef Fbo::GetTex()
	{
		if(!tex)
		{
			tex = g->_CreateTexForFBO(w, h, this);
			E_ASSERT(tex);
			if(!g_is_opengl)
			{
#ifdef E_CFG_DIRECT3D
				// GetSurfaceLevel will increase the Ref count
				IDirect3DSurface9 * p;
				DXTEX(tex->GetNative())->GetSurfaceLevel(0, &p);
				native = (uintx)p;
#endif
			}
		}
		return tex;
	}

	void Fbo::_DeleteTex()
	{
		if(!g_is_opengl && native)
		{
#ifdef E_CFG_DIRECT3D
			DXFBO(native)->Release();
			native = 0;
#endif
		}
		tex.Detach();
	}
}
