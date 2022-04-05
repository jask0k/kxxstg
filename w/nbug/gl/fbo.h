
#pragma once

#include <nbug/core/def.h>
#include <nbug/gl/tex.h>
#include <nbug/core/ref.h>
#include <nbug/core/debug.h>

namespace e
{
	class GraphicsImp;
	class Fbo : public RefObject
	{
		friend class Graphics;
		friend class GraphicsImp;
		//friend class ExtRef<Fbo>;
		uint id;
		Fbo(GraphicsImp * _g, int _w, int _h);
		~Fbo();
		GraphicsImp * g;
		int w, h;
		TexRef tex;
		uintx native;
		void _DeleteTex(); // for lost device;
		uintx GetNative()
		{
			if(!tex)
			{
				GetTex();
			}
			return native;
		}
	public:
		TexRef GetTex();
		uint GetID() const
		{ return id; }
	};

	typedef Ref<Fbo> FboRef;
}
