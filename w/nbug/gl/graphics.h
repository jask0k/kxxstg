
#pragma once 

#include <nbug/core/def.h>
#include <nbug/core/file.h>
#include <nbug/core/str.h>
#include <nbug/gl/color.h>
#include <nbug/gl/graphics_def.h>
#include <nbug/gl/tex.h>
#include <nbug/gl/fbo.h>
#include <nbug/gl/shader.h>
#include <nbug/tl/map.h>
#include <nbug/ex/path_indexer.h>

namespace e
{
	typedef TexLoader * (* LOAD_TEX_FUNC)(void * _context, const string & _name);
	class Callback;
	class Font;
	class Win;

	class Graphics;
	class TexPool
	{
		friend class Graphics;
		Graphics * g;
		LOAD_TEX_FUNC load_tex_func;
		void * load_tex_func_context;

		PathIndexer * indexer;
		Map<stringa, Tex*> pool;

		TexPool(Graphics * _g, const Path & _folder, void * _context, LOAD_TEX_FUNC _func);
		~TexPool();
	public:
		Tex * Load(const string & _name, bool _delay = false);
		PathIndexer * GetIndexer()
		{ return indexer; }
		void clear(bool _force_clear_all = false);
	};

	class Graphics
	{
		TexPool * tex_pool;
	public:
		TexPool * GetTexPool()
		{ return tex_pool; }

		void InitTexPool(const Path & _folder, void * _context=0, LOAD_TEX_FUNC _func=0);

		FboRef CreateFbo(int _w, int _h);
		bool SetFbo(Fbo * _fbo);

		Tex * LoadTexFromFile(const Path & _path, bool _delay = false);
		Tex * LoadTexFromPool(const string & _name, bool _delay = false);
		Tex * LoadTexFromLoader(TexLoaderRef _loader, bool _delay = false);

		void BindTex(Tex * _tex);
		bool BeginScene();
		void EndScene();

		static InternalColor RGBAtoInternalColor(const RGBA & _r);
		static RGBA InternalColortoRGBA(InternalColor _c);
		void SetColor(InternalColor _c);
		void SetColor(const RGBA & _c);
		void SetColor(float _r, float _g, float _b, float _a)
		{ 
			RGBA rgba = {_r, _g, _b, _a}; 
			SetColor(rgba);
		}
		void SetClearColor(InternalColor _c);
		void SetClearColor(const RGBA & _c);
		void SetClearColor(float _r, float _g, float _b, float _a)
		{
			RGBA rgba = {_r, _g, _b, _a}; 
			SetClearColor(rgba);
		}

		void SetTexMode(TexMode _mode);
		TexMode GetTex() const
		{ return texMode; }
		void SetBlendMode(BlendMode _bm);
		BlendMode GetBlendMode() const
		{ return blendMode; }

		// matrix stack
		//void SetActiveMatrixStack(MatrixStackID _mt);
		//MatrixStackID GetActiveMatrixStack() const
		//{ return currentMatrixStack; }
		void TranslateMatrix(float _x, float _y, float _z = 0);
		void RotateMatrix(float _angle, float _x, float _y, float _z);
		void ScaleMatrix(float _x, float _y, float _z = 0);
		void MultMatrix(Matrix4 & _mat);
		void PushMatrix();
		void PopMatrix();
		void SetMatrix(const Matrix4 & _mat);
		void GetMatrix(Matrix4 & _mat) const;

		void SetProjectViewMatrix(const Matrix4 & _project, const Matrix4 & _view);
	//	void GetProjectViewMatrix(Matrix4 & _project, Matrix4 & _view) const;

		// calc perspective (camera) matrix
		void CalcOrtho2D(Matrix4 & _mat, float _left, float _right, float _bottom, float _top);
		void CalcPerspective(Matrix4 & _mat, float _fovyArc, float _aspect, float _near, float _far);

		// calc view matrix
		void CalcLookAt(Matrix4 & _mat, const Vector3 & _eye, const Vector3 & _lookAt, const Vector3 & _up);
//		void CalcLookAt1(Matrix4 & _mat, const Vector3 & _eye, const Vector3 & _lookAt, const Vector3 & _up);

		void GetViewport(int & _x, int & _y, int & _w, int & _h) const;
		void SetViewport(int _x, int _y, int _w, int _h);

		// font and text
		void SetFont(Font * _font);
		//Font * LoadFont(const Path & _path, uint _size = 16);

		// "/foo/bar.tff": load from file system.
		// "foo-bar":      treat as font name, load by system.
		// "foo-bar.tff":  search in known folders.
		Font * LoadFont(const string & _name, int _size, bool _bold = false);
		void DrawString(float _x, float _y, const Char * _buf, int _len);
		void DrawString(float _x, float _y, const string & _text)
		{ DrawString(_x, _y, _text.c_str(), _text.length());	}
		void DrawString(float _x0, float _y0, float _x1, float _y1, const Char * _buf, int _len);
		void DrawString(float _x0, float _y0, float _x1, float _y1, const string & _text)
		{ DrawString(_x0, _y0, _x1, _y1, _text.c_str(), _text.length());	}

		// (x0,y0)-----+
		//  |          |
		//  +-----(x1,y1)
		void DrawQuad(float _x0, float _y0, float _x1, float _y1,
						float _tx0 = 0, float _ty0 = 0, float _tx1 = 1, float _ty1 = 1);

		// 0 - 3
		// |   |
		// 1 - 2
		void DrawQuad(float _x0, float _y0, float _z0, float _tx0, float _ty0,
						float _x1, float _y1, float _z1, float _tx1, float _ty1,
						float _x2, float _y2, float _z2, float _tx2, float _ty2,
						float _x3, float _y3, float _z3, float _tx3, float _ty3);

		//  0-2-4-6  ...
		//  |/|/|/|  ...
		//  1-3-5-7  ...
		void DrawQuadStripX(Vector2 _vertices[], size_t _n);
		
		// 0-1
		// |/|
		// 2-3
		// |/|
		// 4-5
		// ...
		void DrawQuadStripY(Vector2 _vertices[], size_t _n);

		void ClearBuffer(bool _color, bool _z, bool _stencil);
		
		void SetFogParam(float _r, float _g, float _b, float _density, float _start, float _end, int _mode);

		void SetAmbient(const RGBA & _ambient);
		void SetMaterial(Material & _material);
		void SetLight(int _index, const Light * _light);
//		void EnableLight(int _index, bool _enable);

		//void SetWireFrameMode(bool _wired);
		//bool GetWireFrameMode() const;

		// handle device lost
		bool IsDeviceOperational() const;
		bool TryRestoreDevice();

#ifdef NB_DEBUG
		int debugTotalTextureMemory;
		uint64 DebugRenderOjbectCount(bool _reset);
		uint64 DebugRenderVertexCount(bool _reset);
		uint64 DebugTexSwitchCount(bool _reset);
		uint64 DebugMaterialSwitchCount(bool _reset);
#endif

		bool SetVertexSource(void * _p, uint _stride, int _format, int _vertexCount);
		void DrawPrimitive(Primitive _type, int _firstVertex, int _vertexCount);
		void DrawMultiPrimitive(Primitive _type, int _firstVertices[], int _vertexCounts[], int _objectCount);

		bool GetImage(Image & _pic, int _x, int _y, uint _w, uint _h);
		bool PutImage(Image & _pic, int _x, int _y);

		Shader * LoadShader(const Path & _vertex, const Path & _pixel);

		void SetShader(Shader * _shader);
		void SetUniform(uintx _loc, float _v);
		void SetUniform(uintx _loc, const float * _v, uint _n);
		void SetUniform(uintx _loc, const Vector2 & _v);
		void SetUniform(uintx _loc, const Vector2 * _v, uint _n);
		void SetUniform(uintx _loc, const Vector3 & _v);
		void SetUniform(uintx _loc, const Vector3 * _v, uint _n);
		void SetUniform(uintx _loc, const Vector4 & _v);
		void SetUniform(uintx _loc, const Vector4 * _v, uint _n);
		void SetUniform(uintx _loc, int _v);
		void SetUniform(uintx _loc, const int * _v, uint _n);
		
		void SetErrorCallback(const Callback & _callback);
#ifdef E_CFG_LUA
		static bool Register(lua_State * L);
#endif
		static Graphics * Singleton()
		{
			return singleton;
		}

		enum DeviceType
		{
			OpenGL,
			DirectX,
		};
		static void SetDeviceType(DeviceType _type);
		static DeviceType GetDeviceType();
		string GetHardwareString(); // Nvidia 9600GT
		string GetSoftwareString(); // OpenGL 2.0
		struct Capabilities
		{
			bool frameBufferObject;
			bool vertexShader;
			bool pixelShader;
		};
		const Capabilities & GetCapabilities() const
		{ return capability; }

		Win * GetWin();

		void Enable(GraphicsState _s, bool _enable = true);
		void Disable(GraphicsState _s)
		{ Enable(_s, false); }

		void SetScissor(int _x0, int _y0, int _x1, int _y1);
		void GetScissor(int &_x0, int &_y0, int &_x1, int &_y1);

		// slow functions
		void DrawEllipse(float _xc, float _yc, float _rotate, float _a, float _b, const RGBA & _color, bool fill = true, uint _n = 16);
		void DrawQuad(float _xc, float _yc, float _rotate, float _a, float _b, const RGBA & _color, bool fill = true);
		void DrawTriangle(float _xc, float _yc, float _rotate, float _a, float _b, const RGBA & _color, bool fill = true);
		//void DrawLine(float _x0, float _y0, const RGBA & _color0, float _x1, float _y1, const RGBA & _color1);
		void DrawLine(float _x0, float _y0, float _z0, const RGBA & _color0, float _x1, float _y1, float _z1, const RGBA & _color1);
		void DrawHalfAxisGrid(float _size);
		void CheckAndCommit()
		{
			if(vbObjectCount || needSubmitMatrix)
			{
				_Submit();
			}
		}
		struct RenderState
		{
			bool wire_frame_mode;
			bool lighting;
		};

		const RenderState & GetRenderState() const
		{ return render_state; }
	private:
		RenderState render_state;
		Capabilities capability;
		void _OnCreateDevice();
		void _OnLostDevice();
		Fbo * bindedFbo;
		int saved_viewport_x, saved_viewport_y, saved_viewport_w, saved_viewport_h;
		Tex * bindedTex;
		Font * bindedFont;
		Shader * currentShader;
		BlendMode blendMode;
		TexMode texMode;
		//MatrixStackID currentMatrixStack;
		InternalColor clearColor;
		InternalColor currentColor;
		GraphicsImp * imp;

		Primitive  vbPrimitive;
		union
		{
			uint8 * vbBuf;
			V     * vbBufV;
			VC    * vbBufVC;
			VT    * vbBufVT;
			VCT   * vbBufVCT;
		};

		int   vbVertexType;
		uint  vbStride;
		int   vbVertexCount;
		int   vbVertexBufSize; // relative to vbVertexType
		int * vbObjectFA;
		int * vbObjectCA;
		uint  vbObjectCount;
		bool needSubmitMatrix; // D3D only
		void _SetPrimitiveType(Primitive _newPrimitive);
		void _SetVertexType(int _newVertexType);
		void _Submit();
		Graphics();
		~Graphics();
		friend class Win;
		friend class GraphicsImp;
		friend struct TexImp;
		friend class FontImp;
		//friend class Tex;
		friend class Fbo;
		void _SetViewport(int _x, int _y, int _w, int _h);
		static Graphics * singleton;
	};
}

