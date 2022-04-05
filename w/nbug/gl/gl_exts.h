
#pragma once

#ifdef E_CFG_OPENGL

#define GL_GLEXT_PROTOTYPES
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef NB_LINUX
#   include <GL/glx.h>
#endif

namespace e
{
	bool QueryOpenGLExtension(const char * _name);
	void * LoadOpenGLExtensionFunc(const char * _name);


#ifndef GL_VERSION_2_0
	typedef char GLchar; 
#endif

#ifndef GL_CLAMP_TO_EDGE
#	define GL_CLAMP_TO_EDGE 0x812F 
#endif

extern "C" 
{
//#ifdef GL_ARB_imaging
//	const static bool e_has_GL_ARB_imaging = true;
//#else
//	extern bool e_has_GL_ARB_imaging;
//#endif

#ifdef GL_EXT_framebuffer_object
	static const bool e_has_GL_EXT_framebuffer_object = true;
#else
	extern bool e_has_GL_EXT_framebuffer_object;
#	define GL_FRAMEBUFFER_EXT 0x8D40
#	define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#	define GL_COLOR_ATTACHMENT1_EXT 0x8CE1
	typedef void (APIENTRY * FUNC_glDeleteFramebuffersEXT)(GLsizei, const GLuint *);
	typedef void (APIENTRY * FUNC_glBindFramebufferEXT)(GLenum _target, GLuint);
	typedef void (APIENTRY * FUNC_glFramebufferTexture2DEXT)(GLenum _target, GLenum _attachment, GLenum _textarget, GLuint _texture, GLint _level);
	typedef void (APIENTRY * FUNC_glGenFramebuffersEXT)(GLsizei _n, GLuint * _framebuffers) ;
	typedef GLenum (APIENTRY * FUNC_glCheckFramebufferStatusEXT)(GLenum _target);
	extern FUNC_glDeleteFramebuffersEXT glDeleteFramebuffersEXT;
	extern FUNC_glBindFramebufferEXT glBindFramebufferEXT;
	extern FUNC_glFramebufferTexture2DEXT glFramebufferTexture2DEXT;
	extern FUNC_glGenFramebuffersEXT glGenFramebuffersEXT;
	extern FUNC_glCheckFramebufferStatusEXT glCheckFramebufferStatusEXT;
#endif

#ifdef GL_ARB_point_sprite
	static const bool e_has_GL_ARB_point_sprite = true;
#else
	extern bool e_has_GL_ARB_point_sprite;
#	define GL_POINT_SIZE_MIN_ARB 0x8126
#	define GL_POINT_SIZE_MAX_ARB 0x8127
#	define GL_POINT_FADE_THRESHOLD_SIZE_ARB 0x8128
#	define GL_POINT_DISTANCE_ATTENUATION_ARB 0x8129
#	define GL_POINT_SPRITE_ARB 0x8861
#	define GL_COORD_REPLACE_ARB  0x8862
	typedef void (APIENTRY * FUNC_glPointParameterfvARB)(GLenum _pname, const GLfloat * _params);
	typedef void (APIENTRY * FUNC_glPointParameterfARB)(GLenum _pname, GLfloat _param);
	extern FUNC_glPointParameterfvARB glPointParameterfvARB;
#	define glPointParameterfvARB glPointParameterfvARB
	extern FUNC_glPointParameterfARB glPointParameterfARB;
#	define glPointParameterfARB glPointParameterfARB
#	endif

#ifdef GL_ARB_window_pos
	static const bool e_has_GL_ARB_window_pos = true;
#else
	extern bool e_has_GL_ARB_window_pos;
	typedef void (APIENTRY * FUNC_glWindowPos2i)(GLint _x, GLint _y);
	extern FUNC_glWindowPos2i glWindowPos2i;
#endif

#ifdef GL_ARB_shader_objects
	static const bool e_has_GL_ARB_shader_objects = true;
#else
	extern bool e_has_GL_ARB_shader_objects;
	// shader
	typedef GLuint (APIENTRY * FUNC_glCreateShader)(GLenum _type);
	typedef void (APIENTRY * FUNC_glDeleteShader)(GLuint _shader);
	typedef void (APIENTRY * FUNC_glShaderSource)(GLuint _shader, GLsizei _count, const GLchar ** _string, const GLint * _length);
	typedef void (APIENTRY * FUNC_glCompileShader)(GLuint _shader);
	typedef void (APIENTRY * FUNC_glGetShaderInfoLog)(GLuint _shader, GLsizei _bufSize, GLsizei * _length, GLchar * _infoLog);
	typedef GLuint (APIENTRY * FUNC_glCreateProgram)();
	typedef void (APIENTRY * FUNC_glAttachShader)(GLuint _program, GLuint _shader);
	typedef void (APIENTRY * FUNC_glDetachShader)(GLuint _program, GLuint _shader);
	typedef void (APIENTRY * FUNC_glLinkProgram)(GLuint _program);
	typedef void (APIENTRY * FUNC_glGetProgramInfoLog)(GLuint _program, GLsizei _bufSize, GLsizei * _length, GLchar * _infoLog);
	typedef void (APIENTRY * FUNC_glUseProgram)(GLuint _program);
	typedef void (APIENTRY * FUNC_glDeleteProgram)(GLuint _program);
	typedef void (APIENTRY * FUNC_glGetShaderiv)(GLuint _shader, GLenum _pname, GLint * _params);
	typedef void (APIENTRY * FUNC_glGetProgramiv)(GLuint _program, GLenum _pname, GLint * _params);
	typedef GLint (APIENTRY * FUNC_glGetUniformLocation)(GLuint _program, const GLchar * _name);
//	typedef void (APIENTRY * FUNC_glGetUniformfv)(GLuint _program, GLint location, GLfloat * _params);
	typedef void (APIENTRY * FUNC_glUniform1f)(GLint _location, GLfloat);
	typedef void (APIENTRY * FUNC_glUniform2f)(GLint _location, GLfloat, GLfloat);
	typedef void (APIENTRY * FUNC_glUniform3f)(GLint _location, GLfloat, GLfloat, GLfloat);
	typedef void (APIENTRY * FUNC_glUniform4f)(GLint _location, GLfloat, GLfloat, GLfloat, GLfloat);
	typedef void (APIENTRY * FUNC_glUniformfv)(GLint _location, GLsizei _count, const GLfloat * _value);
	typedef void (APIENTRY * FUNC_glUniform1i)(GLint _location, GLint);
	//typedef void (APIENTRY * FUNC_glUniform2i)(GLint _location, GLint, GLint);
	//typedef void (APIENTRY * FUNC_glUniform3i)(GLint _location, GLint, GLint, GLint);
	//typedef void (APIENTRY * FUNC_glUniform4i)(GLint _location, GLint, GLint, GLint, GLint);
	typedef void (APIENTRY * FUNC_glUniformiv)(GLint _location, GLsizei _count, const GLint * _value);

	extern FUNC_glCreateShader glCreateShader;
	extern FUNC_glDeleteShader glDeleteShader;
	extern FUNC_glShaderSource glShaderSource;
	extern FUNC_glCompileShader glCompileShader;
	extern FUNC_glGetShaderInfoLog glGetShaderInfoLog;
	extern FUNC_glCreateProgram glCreateProgram;
	extern FUNC_glAttachShader glAttachShader;
	extern FUNC_glDetachShader glDetachShader;
	extern FUNC_glLinkProgram glLinkProgram;
	extern FUNC_glGetProgramInfoLog glGetProgramInfoLog;
	extern FUNC_glUseProgram glUseProgram;
	extern FUNC_glDeleteProgram glDeleteProgram;
	extern FUNC_glGetShaderiv glGetShaderiv;
	extern FUNC_glGetProgramiv glGetProgramiv;
	extern FUNC_glGetUniformLocation glGetUniformLocation;
//	extern FUNC_glGetUniformfv glGetUniformfv;
	extern FUNC_glUniform1f glUniform1f;
	extern FUNC_glUniform2f glUniform2f;
	extern FUNC_glUniform3f glUniform3f;
	extern FUNC_glUniform4f glUniform4f;
	extern FUNC_glUniformfv glUniform1fv;
	extern FUNC_glUniformfv glUniform2fv;
	extern FUNC_glUniformfv glUniform3fv;
	extern FUNC_glUniformfv glUniform4fv;
	extern FUNC_glUniform1i glUniform1i;
	extern FUNC_glUniformiv glUniform1iv;

#	define GL_FRAGMENT_SHADER 0x8B30
#	define GL_VERTEX_SHADER 0x8B31
#	define GL_SHADER_TYPE 0x8B4F
#	define GL_COMPILE_STATUS 0x8B81
#	define GL_LINK_STATUS 0x8B82
#	define GL_VALIDATE_STATUS 0x8B83
#	define GL_ATTACHED_SHADERS 0x8B85
#	define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#	define GL_CURRENT_PROGRAM 0x8B8D
#	define GL_INFO_LOG_LENGTH 0x8B84
#	define GL_SHADER_SOURCE_LENGTH 0x8B88
#endif

	extern bool e_has_WGL_EXT_swap_control;
	typedef int (APIENTRY * FUNC_wglSwapIntervalEXT)( int );
	extern FUNC_wglSwapIntervalEXT e_wglSwapIntervalEXT;

	// treat as same things
#	define e_has_GLX_SGI_swap_control e_has_WGL_EXT_swap_control
#	define e_glXSwapIntervalSGI e_wglSwapIntervalEXT

#ifndef GL_VERSION_1_4
	typedef void (APIENTRY * FUNC_glSecondaryColor3fv)(const GLfloat * _value);
	extern FUNC_glSecondaryColor3fv glSecondaryColor3fv;
#endif

} //extern "C"


}

#endif

