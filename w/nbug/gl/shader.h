#ifndef NBUG_GL_SHADER_H
#define NBUG_GL_SHADER_H

#include <nbug/core/ref.h>
#include <nbug/core/debug.h>

namespace e
{
	class Graphics;
	class Shader : public RefObject
	{
		friend class Graphics;
		friend class GraphicsImp;
		Shader();
		~Shader();
		Graphics * g;
		uintx native;
	public:
		uintx GetNative() const
		{ return native; }
		uintx GetUniformLoc(const char * _name) const;
	};

	typedef Ref<Shader> ShaderRef;
}

#endif

