#include "private.h"
#include <nbug/gl/graphics.h>
#include <nbug/gl/graphics_.h>
#include <nbug/gl/fbo.h>
#include <nbug/gl/font/font_.h>
#include <nbug/gl/tex.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/image.h>
#include <nbug/gl/shader.h>


#ifdef E_CFG_OPENGL
#	include <nbug/gl/gl_exts.h>
#endif

namespace e
{
#ifdef E_CFG_OPENGL
	static const GLuint g_gl_primitive_map_table[_E_PRIMITIVE_MAX] =
	{
		GL_POINTS,
		GL_LINES,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
	};
#endif

#ifdef E_CFG_DIRECT3D
	static const D3DPRIMITIVETYPE g_dx_primitive_map_table[_E_PRIMITIVE_MAX] =
	{
		D3DPT_POINTLIST,
		D3DPT_LINELIST,
		D3DPT_LINESTRIP,
		D3DPT_TRIANGLELIST,
		D3DPT_TRIANGLESTRIP,
		D3DPT_TRIANGLEFAN,
	};
#endif



#ifdef E_CFG_DIRECT3D
	extern DWORD g_dxVtFormatTable[];
	extern int   g_dxVtStrideTable[];
	void DXVB::Release()
	{
		E_SAFE_RELEASE(vb);
		size = 0;
	}

	bool DXVB::Alloc(IDirect3DDevice9 * _device, uint _bytes)
	{
		// NB_PROFILE_INCLUDE;
		E_TRACE_LINE(L"[nb] DXVB::Alloc() : [stride = " + string(stride) + L"], _bytes = " + BytesToMKBText(_bytes));
		E_ASSERT(_device);
		if(_bytes <= 0)
		{
			_bytes = 1;
		}
		Release();
		HRESULT hr = _device->CreateVertexBuffer(_bytes
			, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY
			, fvf
			, D3DPOOL_DEFAULT
			, &vb
			, NULL);
		if(SUCCEEDED(hr))
		{
			size = _bytes;
			offset = 0;
			return true;
		}
		else
		{
			size = 0;
			offset = 0;
			return false;
		}
	}

	bool DXVB::Lock(IDirect3DDevice9 * _device, uint _vertexCount)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_vertexCount > 0);
		E_ASSERT(lockedBuf == 0); 
		uint bytes = stride * _vertexCount;
		bool alloc = vb == 0 || size < bytes * 4;
		if(alloc)
		{
			if(!Alloc(_device, bytes > 0x3ff ? bytes * 6 :  0x3ff * 4))
			{
				return false;
			}
		}

		int remain = size - offset;
		if(remain < (int)bytes)
		{
			offset = 0;
		}
		HRESULT hr = vb->Lock(offset
				, bytes
				, (void**)&lockedBuf
				, offset ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);
		lockedOffset = offset;
		offset+= bytes;
		return SUCCEEDED(hr);
	}
#endif


	GraphicsImp::GraphicsImp()
	{
		//loadTexFunc = 0;
		wrapper = 0;
		winW = 0;
		winH = 0;
		viewX = 0;
		viewY = 0;
		viewW = 1;
		viewH = 1;

//		delayLoadTex = false;
#ifdef NB_DEBUG
		isRendering     = false;
		isBufferCleared = false;
		debugRenderOjbectCount = 0;
		debugRenderVertexCount = 0;
		debugTexSwitchCount = 0;
		debugMaterialSwitchCount = 0;
#endif

#ifdef E_CFG_DIRECT3D
		device = 0;
		memset(vb, 0, sizeof(vb));
#endif
//#ifdef E_CFG_OPENGL
//		doubleBuffer = false;
//#endif
		vaBuf    = enew uint8[E_VA_SIZE];
		vaType   = 0;
		vaOffset = 0;
		vav = 0;
		vat = 0;
		vac = 0;
//		fbos = 0;
		fonts = 0;
		//texes = 0;
	}

	GraphicsImp::~GraphicsImp()
	{
		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			for(int i=0; i<= _VT_MASK; i++)
			{
				vb[i].Release();
			}
#endif
		}
		delete[] vaBuf;
//		E_ASSERT(fbos == 0);
		E_ASSERT(fonts == 0);
		//E_ASSERT(texes == 0);
	}


	void GraphicsImp::ReleaseAllTexResource()
	{
		for(size_t i = 1; i < fboTable.table.size(); i++)
		{
			Fbo * p = fboTable.table[i];
			if(p)
			{
				p->_DeleteTex();
			}
		}

		{
			FontImp * p = fonts;
			while(p)
			{
				p->CleanAll();
				p = p->next;
			}
		}

		for(size_t i = 1; i < texTable.table.size(); i++)
		{
			TexImp * p = texTable.table[i];
			if(p)
			{
				p->DeleteNative();
			}
		}
	}

//	void GraphicsImp::DeleteAllNativeTex()
//	{
//		Tex * p = texes;
//		while(p)
//		{
//			p->DeleteNative();
//			p = p->next;
//		}
//	}


//	static inline int FlipToIndex(bool _flipX, bool _flipY)
//	{
//		return _flipX  ? (_flipY ? 3 : 1) : (_flipY ? 2 : 0);
//	}

#ifdef NB_DEBUG
	static void ClearNativeTexureRect(uintx _native, int _x, int _y, int _w, int _h)
	{
//		// NB_PROFILE_EXCLUDE;
		if(g_is_opengl)
		{
#	ifdef E_CFG_OPENGL
			glBindTexture(GL_TEXTURE_2D, _native);
			uint8 * buf = enew uint8[_w * _h * 4];
			for(int i=0; i<_w * _h; i++)
			{
				buf[i*4 + 0] = 0;
				buf[i*4 + 1] = 0;
				buf[i*4 + 2] = 0;
				buf[i*4 + 3] = 0x88;
			}
			glTexSubImage2D(GL_TEXTURE_2D, 0, _x, _y, _w, _h, GL_RGBA, GL_UNSIGNED_BYTE, buf);
			delete[] buf;
#	endif
		}
		else
		{
#	ifdef E_CFG_DIRECT3D
			D3DLOCKED_RECT lockedRect;
			RECT rc;
			rc.left    = _x;
			rc.top     = _y;
			rc.right   = _x + _w - 1;
			rc.bottom  = _y + _h - 1;
			if(SUCCEEDED(DXTEX(_native)->LockRect(0, &lockedRect, &rc, 0)))
			{
				uint8 * dst = (uint8 *)lockedRect.pBits;
				for(int y=0; y<_h; y++)
				{
					for(int x=0; x<_w; x++)
					{
						dst[x*4+0] = 0;
						dst[x*4+1] = 0;
						dst[x*4+2] = 0;
						dst[x*4+3] = 0x88;
					}
					dst+= lockedRect.Pitch;
				}
				DXTEX(_native)->UnlockRect(0);
			}
			else
			{
				E_ASSERT(0);
			}
#	endif
		}
	}
#endif

	static void _CopyImageAddEdge(Image & _dst, uint _x1, uint _y1, uint _w1, uint _h1, Image & _src, uint _x0, uint _y0)
	{
//		// NB_PROFILE_INCLUDE;
		_dst.CopyRect(_x1+1, _y1+1, _src, _x0, _y0, _w1-2, _h1-2);
		// �Ľ�
		uint8 * s;
		uint8 * d;
		s = _src.Get(_x0, _y0);
		d = _dst.Get(_x1, _y1);
		*((uint32*)d) = *((uint32*)s);
		s = _src.Get(_x0+_w1-3, _y0);
		d = _dst.Get(_x1+_w1-1, _y1);
		*((uint32*)d) = *((uint32*)s);
		s = _src.Get(_x0, _y0+_h1-3);
		d = _dst.Get(_x1, _y1+_h1-1);
		*((uint32*)d) = *((uint32*)s);
		s = _src.Get(_x0+_w1-3, _y0+_h1-3);
		d = _dst.Get(_x1+_w1-1, _y1+_h1-1);
		*((uint32*)d) = *((uint32*)s);
		//��
		s = _src.Get(_x0, _y0);
		d = _dst.Get(_x1+1, _y1);
		memcpy(d, s, (_w1-2) * 4);
		//��
		s = _src.Get(_x0, _y0+_h1-3);
		d = _dst.Get(_x1+1, _y1+_h1-1);
		memcpy(d, s, (_w1-2) * 4);
		for(uint y=0; y<_h1-2; y++)
		{
			//��
			//��
			s = _src.Get(_x0, _y0+y);
			d = _dst.Get(_x1, _y1+y+1);
			*((uint32*)d) = *((uint32*)s);
			s = _src.Get(_x0+_w1-3, _y0+y);
			d = _dst.Get(_x1+_w1-1, _y1+y+1);
			*((uint32*)d) = *((uint32*)s);
		}
	}

	void TexImp::CopyFromImage(uint _l, uint _t, uint _r, uint _b, Image & _pic, int _x, int _y, bool _addEdge)
	{
		// NB_PROFILE_INCLUDE;
		int w = _r - _l + 1;
		int h = _b - _t + 1;
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glBindTexture(GL_TEXTURE_2D, native);
			Image buf;
			buf.Alloc(w, h);
			if(_addEdge)
			{
				_CopyImageAddEdge(buf, 0, 0, w, h, _pic, _x, _y);
			}
			else
			{
				buf.CopyRect(_l, _t, _pic, _x, _y, w, h);
			}
			glTexSubImage2D(GL_TEXTURE_2D, 0, _l, _t, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf.data);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			D3DLOCKED_RECT lockedRect;
			RECT rc;
			rc.left    = _l;
			rc.top     = _t;
			rc.right   = _r;
			rc.bottom  = _b;
			//HRESULT hr = native->LockRect(0, &lockedRect, &rc, 0);
			HRESULT hr = DXTEX(native)->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD | D3DLOCK_NO_DIRTY_UPDATE);
			if(SUCCEEDED(hr))
			{
				//uint8 * dst = (uint8 *)lockedRect.pBits;
				Image buf;
				buf.data = (uint8 *)lockedRect.pBits;
				buf.w = lockedRect.Pitch / 4;
				buf.h = 0x7fffffff;
				if(_addEdge)
				{
					_CopyImageAddEdge(buf, 0, 0, w, h, _pic, _x, _y);
				}
				else
				{
					buf.CopyRect(_l, _t, _pic, _x, _y, w, h);
				}
				buf.data = 0;

				DXTEX(native)->UnlockRect(0);
			}
			else
			{
				E_ASSERT(0);
			}
#endif
		}
	}


	void TexImp::MakeErrorTex()
	{
		static Image _pic;
		const int sz = 64;
		if(_pic.data == 0)
		{
			_pic.Alloc(sz, sz);
			//memset(_pic.data, 0xCC, _pic.w * _pic.h *  4);
			uint8 * end = _pic.data + _pic.w*_pic.h*4;
			uint8 * p = _pic.data;
			while(p < end)
			{
				p[0] = 192;
				p[1] = 224;
				p[2] = 192;
				p[3] = 192;
				p+=4;
				if(p < end)
				{
					p[0] = 192;
					p[1] = 192;
					p[2] = 224;
					p[3] = 192;
				}
				p+=4;
				if(p < end)
				{
					p[0] = 255;
					p[1] = 192;
					p[2] = 192;
					p[3] = 192;
				}
				p+=4;
			}
			const int hw = sz / 32;
			const int a = hw + 1;
			const int b = sz - hw - 1;

			for(int i = a; i<=b; i++)
			{
				int x = i;
				for(int y = i-hw; y <= i+hw; y++)
				{
					if(y >= a && y <= b)
					{
						uint8 * p = _pic.Get(x, y);
						p[0] = 255;
						p[1] = 64;
						p[2] = 64;
						p[3] = 255;
					}
				}

				for(int y = sz-1-i-hw; y <= sz-1-i+hw; y++)
				{
					if(y >= a && y <= b)
					{
						uint8 * p = _pic.Get(x, y);
						p[0] = 255;
						p[1] = 64;
						p[2] = 64;
						p[3] = 255;
					}
				}
				
				for(int y = a; y <= a+hw; y++)
				{
					uint8 * p = _pic.Get(x, y);
					p[0] = 255;
					p[1] = 64;
					p[2] = 64;
					p[3] = 255;
					p = _pic.Get(y, x);
					p[0] = 255;
					p[1] = 64;
					p[2] = 64;
					p[3] = 255;
				}

				for(int y = b-hw; y <= b; y++)
				{
					uint8 * p = _pic.Get(x, y);
					p[0] = 255;
					p[1] = 64;
					p[2] = 64;
					p[3] = 255;
					p = _pic.Get(y, x);
					p[0] = 255;
					p[1] = 64;
					p[2] = 64;
					p[3] = 255;
				}

			}
			if(!g_is_opengl)
			{
				_pic.SwapChannel(0, 2);
			}
		}

		if(CreateNative(_pic.w, _pic.h, false, false))
		{
			CopyFromImage(0, 0, _pic.w-1, _pic.h-1, _pic, 0, 0, false);
		}
	}

	TexImp::TexImp(GraphicsImp * _g)
	{
		g = _g;
		native = 0;
		id = g->texTable.Add(this);
	}

	TexImp::~TexImp()
	{
		if(native)
		{
			DeleteNative();
		}
		g->texTable.Remove(id);
	}

	ErrorTexLoader::ErrorTexLoader(const string & _name)
	{
#ifdef NB_DEBUG
		this->dbg_name = _name;
#endif
	}

	bool ErrorTexLoader::Load(Image & _pic)
	{
		return false;
	}

	ImageTexLoader::ImageTexLoader()
	{}

	ImageTexLoader::ImageTexLoader(Image & _src)
		: src(_src)
	{}

	bool ImageTexLoader::Load(Image & _pic)
	{
		if(src.data)
		{
			_pic = src;
			return true;
		}
		else
		{
			message(L"[nb] (WW) ImageTexLoader::Load(): No image data."); 
			return false;
		}
	}


	FileTexLoader::FileTexLoader(const Path & _path)
		: path(_path)
	{}

	bool FileTexLoader::Load(Image & _pic)
	{
		if(path.IsValid())
		{
			if(_pic.Load(path))
			{
				return true;
			}
			else
			{
				message(L"[nb] (WW) FileTexLoader::Load(): Load failed: " + path.GetBaseName(true)); 
			}
		}
		return false;
	}

	Tex::Tex(TexImp * _imp, bool _flip_x, bool _flip_y)
		: imp(_imp)
		, flip_x(_flip_x)
		, flip_y(_flip_y)
	{
		imp->AddRef();
	}

	Tex::~Tex()
	{
		imp->Release();
	}

	Tex * Tex::Clone()
	{
		return enew Tex(imp, flip_x, flip_y);
	}

	void Tex::Load()
	{
		if(imp->native == 0 && imp->loader)
		{
			imp->Load();
		}
	}

	bool TexImp::CreateNative(int _w, int _h, bool _renderTarget, bool _clear)
	{
		E_ASSERT(native == 0);
		w = 64;
		h = 64;

		E_ASSERT(g);
		E_ASSERT(g->isDeviceOperational);

		DeleteNative();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			GLuint texture;
			glGenTextures(1, &texture);
			if(texture == 0)
			{
				E_ASSERT(0);
				return false;
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			if(_clear)
			{
				Image pic;
				pic.Alloc(_w, _h);
				pic.Fill(0x00ffffff);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic.data);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			}
			if(!glIsTexture(texture))
			{
				E_ASSERT(0);
				return false;
			}
			native = texture;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			LPDIRECT3DTEXTURE9 texture;
			//HRESULT hr = g_D3DXCreateTexture(_g->device
			//	, _w
			//	, _h
			//	, 1
			//	, _renderTarget ? D3DUSAGE_RENDERTARGET : D3DUSAGE_DYNAMIC
			//	, D3DFMT_A8R8G8B8
			//	, D3DPOOL_DEFAULT
			//	, &texture);

			HRESULT hr = g->device->CreateTexture(_w
				, _h
				, 1
				, _renderTarget ? D3DUSAGE_RENDERTARGET : D3DUSAGE_DYNAMIC
				, D3DFMT_A8R8G8B8
				, D3DPOOL_DEFAULT
				, &texture
				, 0);
			if(FAILED(hr))
			{
				E_ASSERT(0);
				return false;
			}
			native = (uintx)texture;

			if(_clear)
			{
				//RECT rect;
				//rect.left = 0;
				//rect.top = 0;
				//rect.right = _w - 1;
				//rect.bottom = _h - 1;
				D3DLOCKED_RECT lockedRect;
				if(SUCCEEDED(DXTEX(native)->LockRect(0, &lockedRect, NULL, 0)))
				{
					uint32 * p = (uint32 *)lockedRect.pBits;
					//memset(lockedRect.pBits, 0x00, _w * _h * 4);
					for(int i=0; i< _w * _h; i++)
					{
						p[i] = 0x00ffffff;
					}
					DXTEX(native)->UnlockRect(0);
				}
				else
				{
					E_ASSERT(0);
				}
			}
#endif
		}

		w = _w;
		h = _h;
#ifdef NB_DEBUG
		int sz = _w * _h * 4;
		g->wrapper->debugTotalTextureMemory+= sz;
#endif
		return true;
	}

	void TexImp::DeleteNative()
	{
		E_ASSERT(g);
		if(native)
		{
#ifdef NB_DEBUG
			// E_TRACE_LINE(L"[nb] Destroy tex: \"" + this->name + L"\"");
			int sz = w * h * 4;
			g->wrapper->debugTotalTextureMemory-= sz;
#endif

			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				GLuint v1 = native;
				glDeleteTextures(1, &v1);
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				ULONG n = DXTEX(native)->Release();
				E_ASSERT(n == 0);
#endif
			}
			native = 0;
		}
	}

	void TexImp::Load()
	{
		E_ASSERT(g->isDeviceOperational);
		E_ASSERT(native == 0);
		E_ASSERT(loader);
		Image pic;
		if(loader->Load(pic))
		{
			pic.CorrectNonzeroTransparent();
			if(!g_is_opengl)
			{
				pic.SwapChannel(0, 2);
			}

			if(CreateNative(pic.w, pic.h, false, false))
			{
				CopyFromImage(0, 0, pic.w-1, pic.h-1, pic, 0, 0, false);
			}
			else
			{
				throw(NB_SRC_LOC "Failed to create native texture");
			}
		}
		else
		{
#ifdef NB_DEBUG
			E_TRACE_LINE(L"[nb] (WW) Load tex failed: \"" + this->name + L"\"");
#endif
			MakeErrorTex();
			loader = 0;
		}
	}



	int Tex::W()
	{
		if(imp->native == 0 && imp->loader)
		{
			imp->Load();
		}
		return imp->w;
	}

	int Tex::H()
	{
		if(imp->native == 0 && imp->loader)
		{
			imp->Load();
		}
		return imp->h;
	}


	uintx Tex::GetNative()
	{
		if(imp->native == 0 && imp->loader)
		{
			imp->Load();
		}
		return imp->native;
	}

	bool GraphicsImp::SetVertexSource(void * _p, uint _stride, int _format, int _vertexCount)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(this->isDeviceOperational);
		E_ASSERT(_p);
		E_ASSERT(_stride > 0);
		E_ASSERT(_format>=0 && _format<= _VT_MASK);
		E_ASSERT(_vertexCount > 0);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL

			const static int _offset_n[_VT_MASK + 1] =
			{
				0, // V
				sizeof(Vector3), // VN
				0, // VC
				sizeof(Vector3), // VNC
				0, // VT
				sizeof(Vector3), // VNT
				0, // VCT
				sizeof(Vector3), //VNCT
			};

			const static int _offset_c[_VT_MASK + 1] =
			{
				0, // V
				0, // VN
				sizeof(Vector3),// VC
				sizeof(Vector3) + sizeof(Vector3),  //VNC
				0, // VT
				0, // VNT
				sizeof(Vector3), // VCT
				sizeof(Vector3) + sizeof(Vector3), //VNCT
			};

			const static int _offset_t[_VT_MASK + 1] =
			{
				0,
				0,
				0,
				0,
				sizeof(Vector3), // VT
				sizeof(Vector3) + sizeof(Vector3), // VNT
				sizeof(Vector3) + sizeof(InternalColor), // VCT
				sizeof(Vector3) + sizeof(Vector2) + sizeof(InternalColor),//VNCT
			};

			uint8 * p = (uint8*)_p;
			//vsBuf_v = (Vector3*)p;
			glVertexPointer(3, GL_FLOAT, _stride, p);
			if(_format & VT_C)
			{
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(4, GL_UNSIGNED_BYTE, _stride, p + _offset_c[_format]);
			}
			else
			{
				glDisableClientState(GL_COLOR_ARRAY);
			}

			if(_format & VT_N)
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, _stride, p + _offset_n[_format]);
			}
			else
			{
				glDisableClientState(GL_NORMAL_ARRAY);
			}

			if(_format & VT_T)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, _stride, p + _offset_t[_format]);
			}
			else
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}


#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			DXVB & vb = this->vb[_format];
			if(!vb.Lock(this->device, _vertexCount))
			{
				E_ASSERT(0);
				return false;
			}

			uint8 * q = vb.lockedBuf;
			uint8 * p = (uint8*)_p;

			if(_stride == vb.stride)
			{
				memcpy(q, p, _vertexCount * _stride);
			}
			else
			{
				E_ASSERT(_stride > vb.stride);
				uint8 * pEnd = p + _vertexCount * _stride;
				while(p < pEnd)
				{
					memcpy(q, p, vb.stride);
					p+= _stride;
					q+= vb.stride;
				}
			}
			vb.Unlock();
			this->device->SetStreamSource(0, vb.vb, vb.lockedOffset, vb.stride);
			this->device->SetFVF(vb.fvf);
#endif
		}
		return true;
	}


	void GraphicsImp::DrawPrimitive(Primitive _type, int _firstVertex, int _vertexCount)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_type >= E_POINTLIST && _type < _E_PRIMITIVE_MAX && _firstVertex >= 0 && _vertexCount >= 1);
#ifdef NB_DEBUG
		debugRenderOjbectCount++;
		debugRenderVertexCount+= _vertexCount;
#endif
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glDrawArrays(g_gl_primitive_map_table[_type], _firstVertex, _vertexCount);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			// POINTLIST,LINELIST,LINESTRIP,TRIANGLELIST,TRIANGLESTRIP,TRIANGLEFAN,
			const static int _subTable[] = {0, 0, 1, 0, 2, 2};
			const static int _divTable[] = {1, 2, 1, 3, 1, 1};

			E_ASSERT(sizeof(_subTable) == _E_PRIMITIVE_MAX * sizeof(int)
				&& sizeof(_divTable) == sizeof(_subTable));

			int pn = (_vertexCount - _subTable[_type]) / _divTable[_type];
			E_ASSERT(pn > 0);
			this->device->DrawPrimitive(g_dx_primitive_map_table[_type], _firstVertex, pn);
#endif
		}
	}

	void GraphicsImp::DrawMultiPrimitive(Primitive _type, int _firstVertices[], int _vertexCounts[], int _objectCount)
	{
		E_ASSERT(_objectCount > 0);
		for(int i=0; i<_objectCount; i++)
		{
			DrawPrimitive(_type, _firstVertices[i], _vertexCounts[i]);
		}
	}

	bool Tex::GetImage(Image & _pic)
	{
		if(imp->GetImage(_pic))
		{
			if(this->flip_x)
			{
				_pic.FlipH();
			}
			if(this->flip_y)
			{
				_pic.FlipV();
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	bool TexImp::GetImage(Image & _pic)
	{
		_pic.Delete();
		if(native == 0 && loader)
		{
			Load();
		}
		if(native == 0)
		{
			return false;
		}
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			GLint w, h;
			glBindTexture(GL_TEXTURE_2D, native);

			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
			_pic.Alloc(w, h);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pic.data);

			Graphics * g = this->g->wrapper;
			if(g->bindedTex)
			{
				glBindTexture(GL_TEXTURE_2D,g->bindedTex->GetNative());
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			return true;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D

			HRESULT hr;
			D3DSURFACE_DESC desc;
			hr = DXTEX(native)->GetLevelDesc(0, &desc);
			if(FAILED(hr))
			{
				g->ReportGraphicsError(hr);
				return false;
			}

			_pic.Alloc(desc.Width, desc.Height);

			bool ret = false;

			// try lock
			D3DLOCKED_RECT lockedRect;
			hr = DXTEX(native)->LockRect(0, &lockedRect, NULL, 0);
			if(SUCCEEDED(hr))
			{
				uint8 * dst = (uint8 *)lockedRect.pBits;
				memcpy(_pic.data, dst, desc.Width * desc.Height * 4);
				DXTEX(native)->UnlockRect(0);
				_pic.SwapChannel(0, 2);
				ret = true;
			}
			else
			{
				// lock failed,
				E_ASSERT((desc.Usage & D3DUSAGE_RENDERTARGET) || (desc.Pool == 0));

				IDirect3DSurface9 * dstSurface;
				hr = g->device->CreateOffscreenPlainSurface(desc.Width
					, desc.Height
					, D3DFMT_A8R8G8B8
					, D3DPOOL_SYSTEMMEM
					, &dstSurface
					, 0);
				if(SUCCEEDED(hr) && dstSurface)
				{
					IDirect3DSurface9 * srcSurface;
					DXTEX(native)->GetSurfaceLevel(0, &srcSurface);
					if(srcSurface)
					{
						hr = g->device->GetRenderTargetData(srcSurface, dstSurface);
						if(SUCCEEDED(hr))
						{
							D3DLOCKED_RECT lockedRect;
							hr = dstSurface->LockRect(&lockedRect, NULL, 0);
							if(SUCCEEDED(hr))
							{
								uint8 * dst = (uint8 *)lockedRect.pBits;
								memcpy(_pic.data, dst, desc.Width * desc.Height * 4);
								dstSurface->UnlockRect();
								_pic.SwapChannel(0, 2);
								ret = true;
							}
							else
							{
								g->ReportGraphicsError(hr);
							}
						}
						else
						{
							g->ReportGraphicsError(hr);
						}
					}

					E_SAFE_RELEASE(srcSurface);
					E_SAFE_RELEASE(dstSurface);
				}
				else
				{
					g->ReportGraphicsError(hr);
				}

			}

			return ret;
#endif
		}
	}

	TexRef GraphicsImp::_CreateGlyphTex(int _w, int _h, void * _font, int _index)
	{
		TexImp * p = enew TexImp(this);
#ifdef NB_DEBUG
		p->name = "FONT:" + PointerToHex(_font) + string(_index);
#endif
		try
		{
			p->CreateNative(_w, _h, false, true);
		}
		catch(...)
		{
			delete p;
			return 0;
		}

		Tex * tex = enew Tex(p, false, false);
		return tex;
	}

	TexRef GraphicsImp::_CreateTexForFBO(int _w, int _h, void * _fb)
	{
		TexImp * p = enew TexImp(this);
#ifdef NB_DEBUG
		p->name = "FBO:" + PointerToHex(_fb);
#endif
		try
		{
			p->CreateNative(_w, _h, true, false);
		}
		catch(...)
		{
			delete p;
			return 0;
		}
		
		Tex * tex = g_is_opengl ? enew Tex(p, false, true) :  enew Tex(p, false, false);
		return tex;
	}
	
	Shader::Shader()
	{
		g = 0;
		native = 0;
	}

	Shader::~Shader()
	{
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glDeleteProgram(native);
#endif
		}
		else
		{
		}
	}

	uintx Shader::GetUniformLoc(const char * _name) const
	{
		uintx ret = 0;
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			ret = glGetUniformLocation(native,_name);
#endif
		}
		else
		{
		}
		return ret;
	}


#ifdef E_CFG_DIRECT3D
	void GraphicsImp::ReportGraphicsError(HRESULT _hr)
	{
		if(!errorCallback || SUCCEEDED(_hr))
		{
			return;
		}
		string s = string(uintx(_hr));
		errorCallback((void*)&s); // TODO: get rid off void*
	}
#endif


#ifdef E_CFG_OPENGL
	void GraphicsImp::ReportGraphicsError(int _err)
	{
		if(!errorCallback || _err == GL_NO_ERROR)
		{
			return;
		}
		string s;
		switch(_err)
		{
		case GL_INVALID_ENUM:
			s = "GL_INVALID_ENUM An unacceptable value is specified for an enumerated argument.";
			break;
		case GL_INVALID_VALUE:
			s = "GL_INVALID_VALUE A numeric argument is out of range.";
			break;
		case GL_INVALID_OPERATION:
			s = "GL_INVALID_OPERATION The specified operation is not allowed in the current state.";
			break;
		case GL_STACK_OVERFLOW:
			s = "GL_STACK_OVERFLOW This function would cause a stack overflow.";
			break;
		case GL_STACK_UNDERFLOW:
			s = "GL_STACK_UNDERFLOW This function would cause a stack underflow.";
			break;
		case GL_OUT_OF_MEMORY:
			s = "GL_OUT_OF_MEMORY There is not enough memory left to execute the function.";
			break;
		}
		errorCallback(&s);
	}
#endif

#ifdef E_CFG_LUA
#	define CLASSNAME "Tex"

	Tex * Tex::Lua_ToTex(lua_State * L, int _index)
	{
		int type = lua_type(L, _index);
		if(type == LUA_TUSERDATA)
		{
			void *ud = lua_touserdata(L, _index);
			if(ud == 0)
			{
				luaL_typerror(L, _index, CLASSNAME);
			}
			return *(Tex**)ud;
		}
		else if(type != LUA_TNIL)
		{
			luaL_typerror(L, _index, CLASSNAME);
		}

		return 0;
	}

	static int Tex_gc(lua_State * L)
	{
		Tex * tex = Tex::Lua_ToTex(L, 1);
		if(tex)
		{
			tex->Release();
		}
		return 0;
	}

	static int Tex_Size(lua_State * L)
	{
		Tex * tex = Tex::Lua_ToTex(L, 1);
		int w = tex ? tex->W() : 0;
		lua_pushinteger(L, w);
		int h = tex ? tex->H() : 0;
		lua_pushinteger(L, h);
		return 2;
	}


	static const luaL_reg Tex_methods[] = 
	{
		{"Size", &Tex_Size},
		{0, 0}
	};

	int LuaSetTex(lua_State * L)
	{
		Tex * tex = *(Tex**) lua_touserdata(L, 1);
		Graphics::Singleton()->BindTex(tex);
		return 0;
	}

	int LuaLoadTex(lua_State * L)
	{
		E_ASSERT(lua_gettop(L) == 1);
		const char * p = lua_tostring(L, 1);
		Tex * tex = Graphics::Singleton()->LoadTex(p);
		if(tex)
		{
			tex->AddRef();
			void * ud = lua_newuserdata(L, sizeof(Tex *));
			*(Tex**)ud = tex;
			luaL_getmetatable(L, CLASSNAME);
			lua_setmetatable(L, -2);
		}
		else
		{
			lua_pushnil(L);
		}
		return 1;
	}

	void Tex::Lua_PushTex(lua_State * L, Tex * _p)
	{
		if(_p)
		{
			_p->AddRef();
			void * ud = lua_newuserdata(L, sizeof(Tex *));
			*(Tex**)ud = _p;
			luaL_getmetatable(L, CLASSNAME);
			lua_setmetatable(L, -2);
		}
		else
		{
			lua_pushnil(L);
		}
	}

	bool Tex::Register(lua_State * L)
	{
		lua_newtable(L);
		int methodtable = lua_gettop(L);
		luaL_newmetatable(L, CLASSNAME);
		int metatable = lua_gettop(L);

		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, methodtable);
		lua_settable(L, metatable); 

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methodtable);
		lua_settable(L, metatable);

		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, Tex_gc);
		lua_settable(L, metatable);

		lua_pop(L, 1);

		luaL_openlib(L, 0, Tex_methods, 0);
		lua_pop(L, 1);

		lua_register(L, "Tex", &LuaLoadTex);
		lua_register(L, "BindTex", &LuaSetTex);

		return true;
	}
#endif


}
