
#pragma once

#include <nbug/core/def.h>
#include <nbug/core/ref.h>
#include <nbug/core/debug.h>
#include <nbug/gl/image.h>

namespace e
{
	class TexLoader : public RefObject
	{
	public:
#ifdef NB_DEBUG
		string dbg_name;
#endif
		virtual bool Load(Image & _pic) = 0;
	};
	typedef Ref<TexLoader> TexLoaderRef;

	class ErrorTexLoader : public TexLoader
	{
	public:
		ErrorTexLoader(const string & _name);
		bool Load(Image & _pic) override;
	};

	class ImageTexLoader : public TexLoader
	{
	public:
		Image src;
		ImageTexLoader();
		ImageTexLoader(Image & _src);
		bool Load(Image & _pic) override;
	};

	class FileTexLoader : public TexLoader
	{
		Path path;
	public:
		FileTexLoader(const Path & _path);
		bool Load(Image & _pic) override;
	};

	class GraphicsImp;
	struct TexImp;
	class Tex : public RefObject
	{
		TexImp * imp;
		Tex(TexImp * _imp, bool _flip_x, bool _flip_y);
		~Tex();
		Tex(const Tex & _r);
		const Tex & operator=(const Tex & _r);
		friend class GraphicsImp;
		friend class Graphics;
	public:
		bool flip_x;
		bool flip_y;
		Tex * Clone();
		void Load();
		uintx GetNative();
		bool GetImage(Image & _pic);
		int W();
		int H();
#ifdef E_CFG_LUA
		static bool Register(lua_State * L);
		static Tex * Lua_ToTex(lua_State * L, int _index);
		static void  Lua_PushTex(lua_State * L, Tex * _p);
#endif
	};

	typedef Ref<Tex> TexRef;
}
