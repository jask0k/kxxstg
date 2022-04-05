
#pragma once

#include <nbug/core/debug_.h>
#include <nbug/core/def.h>
#include <nbug/core/debug.h>
#include <nbug/core/file.h>

namespace e
{
	class Path;
	class Image
	{
	public:
		uint32 w;
		uint32 h;
		uint8 * data;
		Image();
		Image(const Image & _r);
		~Image();

		Image & operator=(const Image & _r);

		bool Load(const Path & _path);
		bool Save(const Path & _path, bool _keep_alpha = true);
		void Delete();
		bool Alloc(uint _w, uint _h);

		uint8 * Get(uint32 _x, uint32 _y)
		{ E_BASIC_ASSERT(_x < w && _y < h);	return data + (_x + _y * w) * 4; }

		void Set(uint32 _x, uint32 _y, uint8 _r, uint8 _g, uint8 _b, uint8 _a = 0xff)
		{ uint8 * p = data + (_x + _y * w) * 4; *p++ = _r; *p++ = _g; *p++ = _b; *p++ = _a; }

		void Set(uint32 _x, uint32 _y, uint32 _dword)
		{ *((uint32*)(data + (_x * _y * w * 4))) = _dword; }

		bool SwapChannel(uint _a, uint _b);
		bool GetSubImage(Image & _sub, uint _x, uint _y, uint _w, uint _h);
		bool CopyRect(uint _x, uint _y, Image & _src, uint _x0, uint _y0, uint _w0, uint _h0);
		void Fill(uint32 _v);
		void FillChannel(int _channel, uint8 _v);

		void FlipV();
		void FlipH();

		void Blur(uint32 _radius);
		void BlurHorizontal(uint32 _radius);
		void BlurVertical(uint32 _radius);

		bool HasNonzeroTransparent();
		void CorrectNonzeroTransparent();

		void TransparentByColor(uint32 _v);
	};
}

