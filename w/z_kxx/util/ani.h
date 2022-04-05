
#pragma once

#include <nbug/gl/tex.h>
#include <nbug/core/file.h>

namespace e
{
	class Graphics;
	class AniImp;

	// Ani is animation construct by multi TexRef
	class Ani
	{
		friend class AniImp;
		AniImp * imp;
		int index; 
		uint timer;
		int loop;
	public:
		Ani() 
		{ imp = 0; }
		Ani(const Ani & _r);
		~Ani()
		{ Detach(); }
		const Ani & operator=(const Ani & _r);
		bool Load(Graphics * _g, uint _fps, const Path & _path);
		void Detach();
		operator bool() const
		{ return imp != 0; }
		TexRef GetTex();
		void Reset();
		bool Step();
		uint GetTotalSpan() const;
	};
}
