
#pragma once 

#include <nbug/tl/map.h>
//#include <nbug/tl/set.h>
#include <nbug/tl/stack.h>
#include <nbug/core/str.h>
#include <nbug/gl/graphics_def.h>
#include <nbug/gl/tex.h>
#include <nbug/ui/win.h>
#include <nbug/core/callback.h>

struct IDirect3DTexture9;
struct IDirect3DSurface9;
struct ID3DXMatrixStack;
struct IDirect3DDevice9;

namespace e
{
	class Win;
	class Callback;
	class FontImp;

#ifdef E_CFG_OPENGL
	static inline GLuint GLTEX(uintx _n)
	{ return _n; }
	static inline GLuint GLFBO(uintx _n)
	{ return _n; }
#endif
#ifdef E_CFG_DIRECT3D
	static inline IDirect3DTexture9 * DXTEX(uintx _n)
	{ return (IDirect3DTexture9 *)(void*)_n;}
	static inline IDirect3DSurface9 * DXFBO(uintx _n)
	{ return (IDirect3DSurface9 *)(void*)_n;}
#endif

#define E_VA_SIZE 10000

#ifdef E_CFG_DIRECT3D
	struct DXVB
	{
		DWORD fvf;
		IDirect3DVertexBuffer9 * vb;
		uint8 * lockedBuf;
		uint lockedOffset;
		uint stride;
		uint size;
		uint offset;
		void Release();
		bool Lock(IDirect3DDevice9 * _device, uint _vertexCount);
		inline void Unlock()
		{
			lockedBuf = 0;
			vb->Unlock();
		}
	private:
		bool Alloc(IDirect3DDevice9 * _device, uint _bytes);
	};
#endif //E_CFG_DIRECT3D

	struct TexImp : public RefObject
	{
#ifdef NB_DEBUG
		string name;
#endif
		int id;
		Tex * next;
		uintx native;
		int w;
		int h;
		GraphicsImp * g;
		TexLoaderRef loader;
		TexImp(GraphicsImp * _g);
		~TexImp();
		bool CreateNative(int _w, int _h, bool _renderTarget, bool _clear);
		void CopyFromImage(uint _l, uint _t, uint _r, uint _b, Image & _pic, int _x, int _y, bool _addEdge);
		void DeleteNative();
		void Load();
		void MakeErrorTex();
		bool GetImage(Image & _pic);
	};

	template <typename T> struct IDPointerTable
	{
		Array<T *> table;
		IDPointerTable()
		{
			table.push_back(0); // 
		}
		uint Add(T * _p)
		{
#ifdef NB_DEBUG
			for(uint i = 0; i < table.size(); i++)
			{
				E_ASSERT(_p != table[i]);
			}
#endif

			uint n = 1; 
			for(; n < table.size(); n++)
			{
				if(table[n] == 0)
				{
					table[n] = _p;
					return n;
				}
			}

			table.push_back(_p);
			return n;
		}

		T * Get(uint _id)
		{
			return _id < table.size() ? table[_id] : 0;
		}

		void Remove(uint _id)
		{
			if(_id < table.size())
			{
				table[_id] = 0;
			}
		}
	};

	class GraphicsImp
	{
	public:
		Win * win;
		Graphics * wrapper;
		int winW;
		int winH;
		int viewX;
		int viewY;
		int viewW;
		int viewH;

#ifdef NB_WINDOWS
#	ifdef E_CFG_OPENGL
		HGLRC hGL;
#	endif
		HDC hDC;
		HWND hWnd;
#endif

#ifdef NB_LINUX
		::GLXContext hGL;
		Window hWnd;
		static void _WindowProc(XEvent & _event);
#endif

#ifdef E_CFG_DIRECT3D
		LPDIRECT3D9 d3d;
		IDirect3DDevice9 * device;
		DXVB vb[_VT_MASK+1];
		//bool ReallocVertexBuffer(uint _newSize);
		D3DPRESENT_PARAMETERS d3dpp; // for reset
		void ReportGraphicsError(HRESULT _hr);

#endif

#ifdef E_CFG_OPENGL
		void ReportGraphicsError(int _err);
#endif
		GraphicsFormat gf;
		// vertex buffer
		uint8 * vaBuf;
		uint32 vaOffset;
		uint32 vaType;
		uint8 * vav;
		uint8 * vac;
		uint8 * vat;

#ifdef NB_DEBUG
		bool isRendering;
		bool isBufferCleared;
		uint64 debugRenderOjbectCount;
		uint64 debugRenderVertexCount;
		uint64 debugTexSwitchCount;
		uint64 debugMaterialSwitchCount;
#endif
		//bool delayLoadTex;
		GraphicsImp();
		~GraphicsImp();
		//void GetDriverVersion();

		FontImp * fonts; // all fonts
		//Tex*  texes; // all texes

//		typedef Map<stringa, TexImp*> TexObjectMap;
//		TexObjectMap texObjectMap; // texture - name table

		IDPointerTable<TexImp> texTable;
		IDPointerTable<Fbo> fboTable;

		void DeleteAllNativeTex();
		void ReleaseAllTexResource();


		IDirect3DSurface9 * pBackBuffer;
		bool isDeviceOperational;

	//	E_LOAD_TEX_FUNC loadTexFunc;
	//	void * loadTexFuncParam;

		bool SetVertexSource(void * _p, uint _stride, int _format, int _vertexCount);
		void DrawPrimitive(Primitive _type, int _firstVertex, int _vertexCount);
		void DrawMultiPrimitive(Primitive _type, int _firstVertices[], int _vertexCounts[], int _objectCount);

		TexRef _CreateTexForFBO(int _w, int _h, void * _fb);
		TexRef _CreateGlyphTex(int _w, int _h, void * _font, int _index);
		// ID3DXMatrixStack * matrixStack[2];
		Stack<Matrix4> model_matrix_stack;
		Matrix4 project_matrix;
		Matrix4 view_matrix;
		Callback errorCallback;

	};
}

