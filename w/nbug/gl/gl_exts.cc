
#include "private.h"

#ifdef E_CFG_OPENGL

#include <nbug/core/debug.h>
#include <nbug/tl/str_array.h>

namespace e
{
	extern "C" 
	{
#ifndef GL_EXT_framebuffer_object
		bool e_has_GL_EXT_framebuffer_object = false;
		FUNC_glDeleteFramebuffersEXT glDeleteFramebuffersEXT = 0;
		FUNC_glBindFramebufferEXT glBindFramebufferEXT = 0;
		FUNC_glFramebufferTexture2DEXT glFramebufferTexture2DEXT = 0;
		FUNC_glGenFramebuffersEXT glGenFramebuffersEXT = 0;
		FUNC_glCheckFramebufferStatusEXT glCheckFramebufferStatusEXT = 0;
#endif

#ifndef GL_ARB_point_sprite
		bool e_has_GL_ARB_point_sprite = false;
		FUNC_glPointParameterfvARB glPointParameterfvARB = 0;
		FUNC_glPointParameterfARB glPointParameterfARB = 0;
#endif

#ifndef GL_ARB_window_pos
		bool e_has_GL_ARB_window_pos = false;
		FUNC_glWindowPos2i glWindowPos2i = 0;
#endif

#ifndef GL_ARB_shader_objects
		bool e_has_GL_ARB_shader_objects = false;
		FUNC_glCreateShader glCreateShader = 0;
		FUNC_glDeleteShader glDeleteShader = 0;
		FUNC_glShaderSource glShaderSource = 0;
		FUNC_glCompileShader glCompileShader = 0;
		FUNC_glGetShaderInfoLog glGetShaderInfoLog = 0;
		FUNC_glCreateProgram glCreateProgram = 0;
		FUNC_glAttachShader glAttachShader = 0;
		FUNC_glDetachShader glDetachShader = 0;
		FUNC_glLinkProgram glLinkProgram = 0;
		FUNC_glGetProgramInfoLog glGetProgramInfoLog = 0;
		FUNC_glUseProgram glUseProgram = 0;
		FUNC_glDeleteProgram glDeleteProgram = 0;
		FUNC_glGetShaderiv glGetShaderiv = 0;
		FUNC_glGetProgramiv glGetProgramiv = 0;
		FUNC_glGetUniformLocation glGetUniformLocation = 0;
//		FUNC_glGetUniformfv glGetUniformfv = 0;
		FUNC_glUniform1f glUniform1f = 0;
		FUNC_glUniform2f glUniform2f = 0;
		FUNC_glUniform3f glUniform3f = 0;
		FUNC_glUniform4f glUniform4f = 0;
		FUNC_glUniformfv glUniform1fv = 0;
		FUNC_glUniformfv glUniform2fv = 0;
		FUNC_glUniformfv glUniform3fv = 0;
		FUNC_glUniformfv glUniform4fv = 0;
		FUNC_glUniform1i glUniform1i = 0;
		FUNC_glUniformiv glUniform1iv = 0;

#endif

		bool e_has_WGL_EXT_swap_control = false;
		FUNC_wglSwapIntervalEXT e_wglSwapIntervalEXT = 0;
#ifndef GL_VERSION_1_4
		FUNC_glSecondaryColor3fv glSecondaryColor3fv = 0;
#endif
	}

	void load_opengl_extensions()
	{
		// NB_PROFILE_INCLUDE;
		static bool _inited = false;;
		if(_inited)
		{
			return;
		}
		_inited = true;
#ifdef E_DISABLE_ALL_GL_EXTS
		return ;
#endif

#ifndef GL_EXT_framebuffer_object
		e_has_GL_EXT_framebuffer_object = QueryOpenGLExtension("GL_EXT_framebuffer_object");
		// QueryOpenGLExtension("GL_ARB_framebuffer_object") 
		if(e_has_GL_EXT_framebuffer_object)
		{
			glDeleteFramebuffersEXT = (FUNC_glDeleteFramebuffersEXT) LoadOpenGLExtensionFunc("glDeleteFramebuffersEXT");
			glBindFramebufferEXT = (FUNC_glBindFramebufferEXT) LoadOpenGLExtensionFunc("glBindFramebufferEXT");
			glFramebufferTexture2DEXT = (FUNC_glFramebufferTexture2DEXT) LoadOpenGLExtensionFunc("glFramebufferTexture2DEXT");
			glGenFramebuffersEXT = (FUNC_glGenFramebuffersEXT) LoadOpenGLExtensionFunc("glGenFramebuffersEXT");
			glCheckFramebufferStatusEXT = (FUNC_glCheckFramebufferStatusEXT) LoadOpenGLExtensionFunc("glCheckFramebufferStatusEXT");
		}
#endif

#ifndef GL_ARB_point_sprite
		e_has_GL_ARB_point_sprite =  QueryOpenGLExtension("GL_ARB_point_sprite");
		if(e_has_GL_ARB_point_sprite)
		{
			glPointParameterfvARB = (FUNC_glPointParameterfvARB) LoadOpenGLExtensionFunc("glPointParameterfvARB");
			glPointParameterfARB = (FUNC_glPointParameterfARB) LoadOpenGLExtensionFunc("glPointParameterfARB");
		}
#endif

#ifndef GL_ARB_window_pos
		e_has_GL_ARB_window_pos = QueryOpenGLExtension("GL_ARB_window_pos");
		if(e_has_GL_ARB_window_pos)
		{
			glWindowPos2i = (FUNC_glWindowPos2i) LoadOpenGLExtensionFunc("glWindowPos2i");
			E_ASSERT(glWindowPos2i != 0);
		}
#endif

#ifndef GL_ARB_shader_objects
		e_has_GL_ARB_shader_objects = QueryOpenGLExtension("GL_ARB_shader_objects");
		if(e_has_GL_ARB_shader_objects)
		{
			glCreateShader       = (FUNC_glCreateShader)       LoadOpenGLExtensionFunc("glCreateShader");
			glDeleteShader       = (FUNC_glDeleteShader)       LoadOpenGLExtensionFunc("glDeleteShader");
			glShaderSource       = (FUNC_glShaderSource)       LoadOpenGLExtensionFunc("glShaderSource");
			glCompileShader      = (FUNC_glCompileShader)      LoadOpenGLExtensionFunc("glCompileShader");
			glGetShaderInfoLog   = (FUNC_glGetShaderInfoLog)   LoadOpenGLExtensionFunc("glGetShaderInfoLog");
			glCreateProgram      = (FUNC_glCreateProgram)      LoadOpenGLExtensionFunc("glCreateProgram");
			glAttachShader       = (FUNC_glAttachShader)       LoadOpenGLExtensionFunc("glAttachShader");
			glDetachShader       = (FUNC_glDetachShader)       LoadOpenGLExtensionFunc("glDetachShader");
			glLinkProgram        = (FUNC_glLinkProgram)        LoadOpenGLExtensionFunc("glLinkProgram");
			glGetProgramInfoLog  = (FUNC_glGetProgramInfoLog)  LoadOpenGLExtensionFunc("glGetProgramInfoLog");
			glUseProgram         = (FUNC_glUseProgram)         LoadOpenGLExtensionFunc("glUseProgram");
			glDeleteProgram      = (FUNC_glDeleteProgram)      LoadOpenGLExtensionFunc("glDeleteProgram");
			glGetShaderiv        = (FUNC_glGetShaderiv)        LoadOpenGLExtensionFunc("glGetShaderiv");
			glGetProgramiv       = (FUNC_glGetProgramiv)       LoadOpenGLExtensionFunc("glGetProgramiv");
			glGetUniformLocation = (FUNC_glGetUniformLocation) LoadOpenGLExtensionFunc("glGetUniformLocation");
			//glGetUniformfv       = (FUNC_glGetUniformfv)       LoadOpenGLExtensionFunc("glGetUniformfv");
			glUniform1f          = (FUNC_glUniform1f)          LoadOpenGLExtensionFunc("glUniform1f");
			glUniform2f          = (FUNC_glUniform2f)          LoadOpenGLExtensionFunc("glUniform2f");
			glUniform3f          = (FUNC_glUniform3f)          LoadOpenGLExtensionFunc("glUniform3f");
			glUniform4f          = (FUNC_glUniform4f)          LoadOpenGLExtensionFunc("glUniform4f");
			glUniform1fv         = (FUNC_glUniformfv)          LoadOpenGLExtensionFunc("glUniform1fv");
			glUniform2fv         = (FUNC_glUniformfv)          LoadOpenGLExtensionFunc("glUniform2fv");
			glUniform3fv         = (FUNC_glUniformfv)          LoadOpenGLExtensionFunc("glUniform3fv");
			glUniform4fv         = (FUNC_glUniformfv)          LoadOpenGLExtensionFunc("glUniform4fv");

			glUniform1i          = (FUNC_glUniform1i)          LoadOpenGLExtensionFunc("glUniform1i");
			glUniform1iv         = (FUNC_glUniformiv)          LoadOpenGLExtensionFunc("glUniform1iv");
		}
#endif

#ifdef NB_WINDOWS
		e_has_WGL_EXT_swap_control = QueryOpenGLExtension("WGL_EXT_swap_control");
		if(e_has_WGL_EXT_swap_control)
		{
			e_wglSwapIntervalEXT = (FUNC_wglSwapIntervalEXT) LoadOpenGLExtensionFunc("wglSwapIntervalEXT");
		}
#else
		e_has_WGL_EXT_swap_control = QueryOpenGLExtension("GLX_SGI_swap_control");
		if(e_has_WGL_EXT_swap_control)
		{
			e_wglSwapIntervalEXT = (FUNC_wglSwapIntervalEXT) LoadOpenGLExtensionFunc("glXSwapIntervalSGI");
		}
#endif

#ifndef GL_VERSION_1_4
		glSecondaryColor3fv = (FUNC_glSecondaryColor3fv) LoadOpenGLExtensionFunc("glSecondaryColor3fv");
#endif
	}

	bool QueryOpenGLExtension(const char * _name)
	{
		const char * p = (const char*) glGetString(GL_EXTENSIONS);
		E_ASSERT(p);
		if(p)
		{
#ifdef NB_CFG_VERBOSE
			static bool _reported = false;
			if(!_reported)
			{
				message(L"[nb] Supported OpenGL Extensions:");
				StringArray a = Split(string(p), L" \t");
				if(a.empty())
				{
					message(L"[nb] \t none.");
				}
				else
				{
					for(size_t i = 0; i < a.size(); i++)
					{
						message(L"[nb] \t " + a[i]);
					}
				}
				_reported = true;
			}
#endif
			const char * p1 = p + strlen(p);
			while(p < p1)
			{
				int n = strcspn(p, " ");
				if(strlen(_name) == n && strncmp(_name, p, n) == 0)
				{
					return true;
				}
				p+= n + 1;
			}
		}
		message(L"[nb] (WW) Failed to load OpenGL extension: " + string(_name));
		return false;
	}

	void * LoadOpenGLExtensionFunc(const char * _name)
	{
		void * ret;
#ifdef NB_WINDOWS
		ret = (void*)wglGetProcAddress(_name);
#endif
#ifdef NB_LINUX
		//return (void*)glXGetProcAddress((const GLubyte*)_name);
		ret = (void*)glXGetProcAddressARB((const GLubyte*)_name);
#endif


		if(ret)
		{
#ifdef NB_CFG_VERBOSE
			message(L"[nb] Load OpenGL func: \"") + string(_name) + string("\" succeeded."));
#endif
		}
		else
		{
			message(L"[nb] (WW) Load OpenGL func: \"" + string(_name) + string("\" failed."));
		}
		return ret;
	}
}

#else
	int  ___dummy_var_supress_warning;
#endif
