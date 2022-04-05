
#pragma once

#include <nbug/gl/tex.h>
#include <nbug/core/file.h>

namespace e
{
	class Graphics;
	class Ani5Imp;
	class Ani5
	{
		friend class Ani5Imp;
		Ani5Imp * imp;
		int pos0;
		int pos1;
		int timer;
	public:
		Ani5()
		{ imp = 0; }
		Ani5(const Ani5 & _r);
		~Ani5();
		TexRef GetTex();
		void Step(int _dir); 
		const Ani5 & operator=(const Ani5 & _r);
		bool Load(Graphics * _g, uint _fps, const Path & _path);
		void Detach();
		operator bool() const
		{ return imp != 0; }
	};

}

