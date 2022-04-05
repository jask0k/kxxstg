#include <typeinfo>
#include "private.h"
#include <nbug/gl/graphics.h>
#include <nbug/gl/graphics_.h>
#include <nbug/gl/font/font_.h>
#include <nbug/core/debug.h>
#include <nbug/gl/image.h>
#ifdef E_CFG_OPENGL
#	include <nbug/gl/gl_exts.h>
#endif

namespace e
{
#if defined(E_CFG_DIRECT3D) && defined(E_CFG_OPENGL)
	bool g_is_opengl = true;
#endif

	void Graphics::SetDeviceType(DeviceType _type)
	{
#if defined(E_CFG_DIRECT3D) && defined(E_CFG_OPENGL)
		g_is_opengl = _type == DirectX ? false : true;
#elif defined(E_CFG_DIRECT3D)
		E_ASSERT( _type == DirectX);
#else
		E_ASSERT( _type == OpenGL);
#endif
	}
	
	Graphics::DeviceType Graphics::GetDeviceType()
	{
		return g_is_opengl ? OpenGL : DirectX;
	}

//	bool Graphics::TryInitDirect3D(string & _err)
//	{
//#	ifdef E_CFG_DIRECT3D
//		if(LoadD3DFunc(_err))
//		{
//#		ifdef E_CFG_OPENGL
//			g_is_opengl = false;
//#		endif 
//		}
//		else
//		{
//#		ifdef E_CFG_OPENGL
//			g_is_opengl = true;
//#		endif 
//		}
//#	endif
//		return !g_is_opengl;
//	}


#ifdef NB_LINUX
	extern Display * _display;
	extern int _screenNum;
	extern Colormap _screenColormap;
#endif

#define E_VB_VERTEX_BUFFER_SIZE 4000
#define E_VB_OBJECT_BUFFER_SIZE 64

	static uint g_vtStrideTable[_VT_MASK+1] = {0};

	void load_opengl_extensions();

	Graphics * Graphics::singleton = 0;

	Graphics::Graphics()
	{
		tex_pool = 0;
		render_state.lighting = false;
		render_state.wire_frame_mode = false;
		imp = enew GraphicsImp;
		imp->win = 0;
		imp->wrapper = this;
//		imp->majorVersion = 0;
//		imp->minorVersion = 0;
		Matrix4 m;
		m.LoadIdentity();
		imp->view_matrix = m;
		imp->project_matrix = m;
		imp->model_matrix_stack.push(m);

		imp->isDeviceOperational = false;
#ifdef E_CFG_OPENGL
		imp->hGL = NULL;
#endif

#ifdef NB_WINDOWS
		imp->hDC = NULL;
#endif
		//imp->device = 0;
		this->bindedTex = 0;
		this->bindedFbo = 0;
		this->bindedFont = 0;
		this->currentShader = 0;
//		this->textureOn = true;
		this->texMode = TM_MODULATE;
//		this->blendOn = true;
		this->blendMode = BM_NORMAL;
		this->clearColor = 0xFF000000;
		this->currentColor = 0xFFFFFFFF;
		this->vbPrimitive = E_POINTLIST;
		this->vbObjectCount = 0;
		this->vbVertexType = 0;
		this->vbStride = sizeof(Vector3);
		this->vbBuf = enew uint8[E_VB_VERTEX_BUFFER_SIZE];
		this->vbObjectFA = enew int[E_VB_OBJECT_BUFFER_SIZE];
		this->vbObjectCA = enew int[E_VB_OBJECT_BUFFER_SIZE];
		this->vbVertexBufSize = E_VB_OBJECT_BUFFER_SIZE;
		this->vbVertexCount = 0;
		this->needSubmitMatrix = false;
	//	this->currentMatrixStack = MT_MODELVIEW;
#ifdef NB_DEBUG
		imp->isRendering = false;
		this->debugTotalTextureMemory = 0;
#endif

		for(uint i=0; i<=_VT_MASK; i++)
		{
			uint & s = g_vtStrideTable[i];
			s = sizeof(Vector3);
			if(i & VT_N)
			{
				s+= sizeof(Vector3);
			}

			if(i & VT_C)
			{
				s+= sizeof(InternalColor);
			}

			if(i & VT_T)
			{
				s+= sizeof(Vector2);
			}

		}

		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			for(DWORD i=0; i<=_VT_MASK; i++)
			{
				DWORD & n = imp->vb[i].fvf;
				imp->vb[i].stride = g_vtStrideTable[i];
				n = D3DFVF_XYZ; // xyz coord aways present
				if(i & VT_N)
				{
					n|= D3DFVF_NORMAL;
				}

				if(i & VT_C)
				{
					n|= D3DFVF_DIFFUSE;
				}

				if(i & VT_T)
				{
					n|= D3DFVF_TEX1;
				}

			}
#endif
		}
	}

	Graphics::~Graphics()
	{
		delete tex_pool;
		_OnLostDevice(); // force delete all native resource
		bindedFbo = 0;
		bindedTex = 0;
		currentShader = 0;
		bindedFont = 0;	
	//	imp->DeleteAllNativeTex();

#ifdef NB_WINDOWS
		if(g_is_opengl)
		{
#	ifdef E_CFG_OPENGL
			if(imp->hGL)
			{
				::wglMakeCurrent(NULL, NULL) ;
				::wglDeleteContext((HGLRC)imp->hGL);
			}
#	endif
		}
#endif

#ifdef NB_LINUX
		::glXMakeCurrent(_display, 0, NULL) ;
		::glXDestroyContext(_display, imp->hGL);
#endif

		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			E_SAFE_RELEASE(imp->pBackBuffer);
			//E_SAFE_RELEASE(imp->matrixStack[0]);
			//E_SAFE_RELEASE(imp->matrixStack[1]);
			E_SAFE_RELEASE(imp->device);
			E_SAFE_RELEASE(imp->d3d);
#endif
		}
		//device->Release();
		delete[] vbBuf;
		delete[] vbObjectFA;
		delete[] vbObjectCA;
		delete imp;
	}

	void Graphics::_OnCreateDevice()
	{
		// NB_PROFILE_INCLUDE;
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
#	ifdef NB_WINDOWS
			::wglMakeCurrent((HDC)imp->hDC, (HGLRC)imp->hGL);
			load_opengl_extensions();
#	endif

#	ifdef NB_LINUX
			::glXMakeCurrent(_display, imp->hWnd, imp->hGL);
			load_opengl_extensions();
#	endif

			if(e_wglSwapIntervalEXT)
			{
				e_wglSwapIntervalEXT(imp->gf.vsync ? 1 : 0); // vsync control
			}
			else
			{
				message(L"[nb] (WW) Failed to set/unset VSync mode.");
			}

			glLogicOp(GL_COPY);

			// lighting
			glEnable(GL_NORMALIZE);

			// GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
			// GL_SEPARATE_SPECULAR_COLOR 0x81FA
			// GL_SINGLE_COLOR 0x81F9
			glLightModeli(0x81F8, 0x81FA); 

			// smoothness
			glShadeModel(GL_SMOOTH);
			glEnable(GL_POLYGON_SMOOTH);
			glEnable(GL_LINE_SMOOTH);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_POLYGON_SMOOTH);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

			// blend
			//glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// texture
			glEnable(GL_TEXTURE_2D);

			// texture mixing
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnableClientState(GL_VERTEX_ARRAY);

			// disable write to alpha channel
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClearDepth(1.0f);
			//glDepthFunc(GL_LESS);

			capability.frameBufferObject = e_has_GL_EXT_framebuffer_object;
			capability.vertexShader = e_has_GL_ARB_shader_objects;
			capability.pixelShader = e_has_GL_ARB_shader_objects;
#endif
		}
		else
		{

#ifdef E_CFG_DIRECT3D
			// z buffer
			imp->device->SetRenderState(D3DRS_ZENABLE, FALSE);

			// cull off
			imp->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );

			// lighting
			imp->device->SetRenderState(D3DRS_LIGHTING, FALSE );
			//imp->device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
			imp->device->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
			//imp->device->SetRenderState(D3DRS_LOCALVIEWER, TRUE );

			// material
			imp->device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
			imp->device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			//imp->device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
			imp->device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

			// multi sample
			imp->device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

			// disable write to alpha channel
			imp->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			imp->device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
			imp->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

			// color blend
			imp->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			imp->device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
			imp->device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

			// texture sample
			imp->device->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0x00000000);
			imp->device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			imp->device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			imp->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			imp->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

			// texture mixing
			imp->device->SetTextureStageState(0, D3DTSS_COLORARG1,D3DTA_TEXTURE);
			imp->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			imp->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);



			imp->device->GetRenderTarget(0, &imp->pBackBuffer);



			// matrix
			//if(imp->matrixStack[0] == 0)
			//{
				//E_ASSERT(imp->matrixStack[1] == 0);
				//g_D3DXCreateMatrixStack(0, &imp->matrixStack[0]);
				//g_D3DXCreateMatrixStack(0, &imp->matrixStack[1]);
				//D3DXMATRIX Identity;
				//((Matrix4&)Identity).LoadIdentity();
				//imp->device->SetTransform(D3DTS_WORLD, &Identity);
			//}

		//	Matrix4 mat;
		//	mat.LoadIdentity();
		//	//Vector3 eye = {0, 0, 0};
		//	//Vector3 lookAt = {0, 0, 1};
		//	//Vector3 up = {0, 0, 1};
		////	this->CalcLookAt(mat, eye, lookAt, up);

		//	imp->device->SetTransform(D3DTS_VIEW, (D3DMATRIX*) &mat);


			capability.frameBufferObject = true;
			capability.vertexShader = true;
			capability.pixelShader = true;
#endif
		}


		E_ASSERT(!imp->isDeviceOperational);
		imp->isDeviceOperational = true;

	}

	void Graphics::_OnLostDevice()
	{
		E_ASSERT(imp->isDeviceOperational);
		imp->isDeviceOperational = false;
		//BindTex(0);
		//SetFbo(0);
		if(bindedTex)
		{
			BindTex(0);
		}
		if(bindedFbo)
		{
			SetFbo(0);
		}
		if(bindedFont)
		{
			SetFont(0);
		}
		if(currentShader)
		{
			SetShader(0);
		}
		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			E_SAFE_RELEASE(imp->pBackBuffer);
			for(int i=0; i<= _VT_MASK; i++)
			{
				imp->vb[i].Release();
				imp->vb[i].offset = 0;
				imp->vb[i].size = 0;
			}
#endif
		}
		imp->ReleaseAllTexResource();
		E_ASSERT(debugTotalTextureMemory == 0);
	}

	Tex * Graphics::LoadTexFromPool(const string & _name, bool _delay)
	{ 
		return tex_pool 
			? tex_pool->Load(_name, _delay) 
			: LoadTexFromLoader(enew ErrorTexLoader(_name), _delay);
	}


	Tex * Graphics::LoadTexFromFile(const Path & _path, bool _delay)
	{
		TexLoader * p = enew FileTexLoader(_path);
#ifdef NB_DEBUG
		p->dbg_name = _path.GetBaseName(false);
#endif
		return LoadTexFromLoader( p, _delay);
	}

	Tex * Graphics::LoadTexFromLoader(TexLoaderRef _loader, bool _delay)
	{
		TexImp * p = enew TexImp(imp);

		if(!_loader)
		{
			_loader = enew ErrorTexLoader("_loader==0");
		}

#ifdef NB_DEBUG
		if(_loader->dbg_name.empty())
		{
			p->name = L"Unnamed " + string(typeid(*_loader).name());
		}
		else
		{
			p->name = _loader->dbg_name;
		}
#endif
		p->loader = _loader;
		if(!_delay)
		{
			p->Load();
		}
		Tex * tex = enew Tex(p, false, false);
		return tex;
	}



	void Graphics::DrawQuad(float _x0, float _y0, float _x1, float _y1, float _tx0, float _ty0, float _tx1, float _ty1)
	{
		// NB_PROFILE_INCLUDE;

		if(vbObjectCount >= E_VB_OBJECT_BUFFER_SIZE || vbVertexCount + 4 >= vbVertexBufSize)
		{
			_Submit();
		}

		Tex * tex = bindedTex;
		_SetPrimitiveType(E_TRIANGLESTRIP);

		if(this->texMode && tex)
		{
			if(texMode == TM_MODULATE)
			{

				float tx0 = tex->flip_x ? 1.0f - _tx0 : _tx0;
				float tx1 = tex->flip_x ? 1.0f - _tx1 : _tx1;
				float ty0 = tex->flip_y ? 1.0f - _ty0 : _ty0;
				float ty1 = tex->flip_y ? 1.0f - _ty1 : _ty1;
//				float tx0 = _tx0;
//				float tx1 = _tx1;
//				float ty0 = _ty0;
//				float ty1 = _ty1;

				_SetVertexType(VCT::FORMAT);
				/*
				int _newVertexType = VCT::FORMAT;
				if(_newVertexType != this->vbVertexType)
				{
					CheckAndCommit();
					// E_ASSERT(_newVertexType < _VT_MASK+1);
					vbVertexType = _newVertexType;
					vbStride = g_vtStrideTable[vbVertexType];
					vbVertexBufSize = E_VB_VERTEX_BUFFER_SIZE / vbStride;
				}
				*/

				//_SetVertexType(3);

				VCT * p = vbBufVCT + vbVertexCount;
				p->v.x = _x0; p->v.y = _y0; p->v.z = 0; 
				p->t.x = tx0; p->t.y = ty0; 
				p->c = currentColor;
				p++;
				p->v.x = _x0; p->v.y = _y1; p->v.z = 0;
				p->t.x = tx0; p->t.y = ty1; 
				p->c = currentColor;
				p++;
				p->v.x = _x1; p->v.y = _y0; p->v.z = 0;
				p->t.x = tx1; p->t.y = ty0; 
				p->c = currentColor;
				p++;
				p->v.x = _x1; p->v.y = _y1; p->v.z = 0; 
				p->t.x = tx1; p->t.y = ty1;
				p->c = currentColor;
			}
			else
			{
				float tx0 = tex->flip_x ? 1.0f - _tx0 : _tx0;
				float tx1 = tex->flip_x ? 1.0f - _tx1 : _tx1;
				float ty0 = tex->flip_y ? 1.0f - _ty0 : _ty0;
				float ty1 = tex->flip_y ? 1.0f - _ty1 : _ty1;
//				float tx0 = _tx0;
//				float tx1 = _tx1;
//				float ty0 = _ty0;
//				float ty1 = _ty1;
				_SetVertexType(VT::FORMAT);
				VT * p = vbBufVT + vbVertexCount;
				p->v.x = _x0; p->v.y = _y0; p->v.z = 0; 
				p->t.x = tx0; p->t.y = ty0; 
				//p->c = currentColor;
				p++;
				p->v.x = _x0; p->v.y = _y1; p->v.z = 0;
				p->t.x = tx0; p->t.y = ty1; 
				//p->c = currentColor;
				p++;
				p->v.x = _x1; p->v.y = _y0; p->v.z = 0;
				p->t.x = tx1; p->t.y = ty0; 
				//p->c = currentColor;
				p++;
				p->v.x = _x1; p->v.y = _y1; p->v.z = 0; 
				p->t.x = tx1; p->t.y = ty1;
				//p->c = currentColor;
			}

		}
		else
		{
			_SetVertexType(VC::FORMAT);
			VC * p = vbBufVC + vbVertexCount;
			p->v.x = _x0; p->v.y = _y0; p->v.z = 0; 
		//	p->t.x = tx0; p->t.y = ty0; 
			p->c = currentColor;
			p++;
			p->v.x = _x0; p->v.y = _y1; p->v.z = 0;
		//	p->t.x = tx0; p->t.y = ty1; 
			p->c = currentColor;
			p++;
			p->v.x = _x1; p->v.y = _y0; p->v.z = 0;
		//	p->t.x = tx1; p->t.y = ty0; 
			p->c = currentColor;
			p++;
			p->v.x = _x1; p->v.y = _y1; p->v.z = 0; 
		//	p->t.x = tx1; p->t.y = ty1;
			p->c = currentColor;
		}
		
		vbObjectFA[vbObjectCount] = vbVertexCount;
		vbObjectCA[vbObjectCount] = 4;
		vbObjectCount++;
		vbVertexCount+= 4;
	}


	void Graphics::DrawQuad(float _x0, float _y0, float _z0, float _tx0, float _ty0,
					float _x1, float _y1, float _z1, float _tx1, float _ty1,
					float _x2, float _y2, float _z2, float _tx2, float _ty2,
					float _x3, float _y3, float _z3, float _tx3, float _ty3)
	{
		// NB_PROFILE_INCLUDE;

		if(vbObjectCount >= E_VB_OBJECT_BUFFER_SIZE || vbVertexCount + 4 >= vbVertexBufSize)
		{
			_Submit();
		}

		Tex * tex = bindedTex;
		_SetPrimitiveType(E_TRIANGLESTRIP);
		if(this->texMode && tex)
		{
			float tx0 = tex->flip_x ? 1.0f - _tx0 : _tx0;
			float tx1 = tex->flip_x ? 1.0f - _tx1 : _tx1;
			float tx2 = tex->flip_x ? 1.0f - _tx2 : _tx2;
			float tx3 = tex->flip_x ? 1.0f - _tx3 : _tx3;
			float ty0 = tex->flip_y ? 1.0f - _ty0 : _ty0;
			float ty1 = tex->flip_y ? 1.0f - _ty1 : _ty1;
			float ty2 = tex->flip_y ? 1.0f - _ty2 : _ty2;
			float ty3 = tex->flip_y ? 1.0f - _ty3 : _ty3;

//			float tx0 = _tx0;
//			float tx1 = _tx1;
//			float tx2 = _tx2;
//			float tx3 = _tx3;
//			float ty0 = _ty0;
//			float ty1 = _ty1;
//			float ty2 = _ty2;
//			float ty3 = _ty3;

			if(texMode == TM_MODULATE)
			{
				_SetVertexType(VCT::FORMAT);
				VCT * p = vbBufVCT + vbVertexCount;
				p->v.x = _x0; p->v.y = _y0; p->v.z = _z0; 
				p->t.x = tx0; p->t.y = ty0; 
				p->c = currentColor;
				p++;
				p->v.x = _x1; p->v.y = _y1; p->v.z = _z1; 
				p->t.x = tx1; p->t.y = ty1; 
				p->c = currentColor;
				p++;
				p->v.x = _x3; p->v.y = _y3; p->v.z = _z3; 
				p->t.x = tx3; p->t.y = ty3;
				p->c = currentColor;
				p++;
				p->v.x = _x2; p->v.y = _y2; p->v.z = _z2; 
				p->t.x = tx2; p->t.y = ty2; 
				p->c = currentColor;
			}
			else
			{
				_SetVertexType(VT::FORMAT);
				VT * p = vbBufVT + vbVertexCount;
				p->v.x = _x0; p->v.y = _y0; p->v.z = _z0; 
				p->t.x = tx0; p->t.y = ty0; 
				//p->c = currentColor;
				p++;
				p->v.x = _x1; p->v.y = _y1; p->v.z = _z1; 
				p->t.x = tx1; p->t.y = ty1; 
				//p->c = currentColor;
				p++;
				p->v.x = _x3; p->v.y = _y3; p->v.z = _z3; 
				p->t.x = tx3; p->t.y = ty3;
				//p->c = currentColor;
				p++;
				p->v.x = _x2; p->v.y = _y2; p->v.z = _z2; 
				p->t.x = tx2; p->t.y = ty2; 
				//p->c = currentColor;
			}

		}
		else
		{
			_SetVertexType(VC::FORMAT);
			VC * p = vbBufVC + vbVertexCount;
			p->v.x = _x0; p->v.y = _y0; p->v.z = _z0; 
		//	p->t.x = tx0; p->t.y = ty0; 
			p->c = currentColor;
			p++;
			p->v.x = _x1; p->v.y = _y1; p->v.z = _z1; 
		//	p->t.x = tx1; p->t.y = ty1; 
			p->c = currentColor;
			p++;
			p->v.x = _x3; p->v.y = _y3; p->v.z = _z3; 
		//	p->t.x = tx3; p->t.y = ty3;
			p->c = currentColor;
			p++;
			p->v.x = _x2; p->v.y = _y2; p->v.z = _z2; 
		//	p->t.x = tx2; p->t.y = ty2; 
			p->c = currentColor;
		}
		
		vbObjectFA[vbObjectCount] = vbVertexCount;
		vbObjectCA[vbObjectCount] = 4;
		vbObjectCount++;
		vbVertexCount+= 4;

	}


	void Graphics::DrawQuadStripX(Vector2 _vertices[], size_t _n)
	{
		// NB_PROFILE_INCLUDE;
		if(_n < 4 || ( _n & 0x01) || _n >= vbVertexBufSize)
		{
			E_ASSERT(0);
			return;
		}

		if(vbObjectCount >= E_VB_OBJECT_BUFFER_SIZE || vbVertexCount + (int)_n >= vbVertexBufSize)
		{
			_Submit();
		}

		_SetPrimitiveType(E_TRIANGLESTRIP);
		Tex * tex = bindedTex;
		if(this->texMode && tex)
		{
			float tx0 = tex->flip_x ? 1.0f : 0.0f;
			float tx1 = tex->flip_x ? 0.0f : 1.0f;
			float ty0 = tex->flip_y ? 1.0f : 0.0f;
			float ty1 = tex->flip_y ? 0.0f : 1.0f;
			if(texMode == TM_MODULATE)
			{
				_SetVertexType(VCT::FORMAT);
				VCT * p = vbBufVCT + vbVertexCount;
				size_t k = _n >> 1;
				float dx = (tx1 - tx0)/ (k-1);
				for(size_t i = 0; i < k; i++)
				{
					int j = i * 2;
					{
						Vector2 & v = _vertices[j];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx0+i*dx; p->t.y = ty0;
						p->c = currentColor;
						p++;
					}
					{
						Vector2 & v = _vertices[j+1];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx0+i*dx; p->t.y = ty1;
						p->c = currentColor;
						p++;
					}
				}				
			}
			else
			{
				_SetVertexType(VT::FORMAT);
				VT * p = vbBufVT + vbVertexCount;
				size_t k = _n >> 1;
				float dx = (tx1 - tx0)/ (k-1);
				for(size_t i = 0; i < k; i++)
				{
					int j = i * 2;
					{
						Vector2 & v = _vertices[j];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx0+i*dx; p->t.y = ty0;
						//p->c = currentColor;
						p++;
					}
					{
						Vector2 & v = _vertices[j+1];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx0+i*dx; p->t.y = ty1;
					//	p->c = currentColor;
						p++;
					}
				}				
			}
		}
		else
		{
			_SetVertexType(VC::FORMAT);
			VC * p = vbBufVC + vbVertexCount;
			size_t k = _n >> 1;
			//float dx = (1.0f - 0.0f)/ (k-1);
			for(size_t i = 0; i < k; i++)
			{
				int j = i * 2;
				{
					Vector2 & v = _vertices[j];
					p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
					//p->t.x = tx0+i*dx; p->t.y = ty0;
					p->c = currentColor;
					p++;
				}
				{
					Vector2 & v = _vertices[j+1];
					p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
					//p->t.x = tx0+i*dx; p->t.y = ty1;
					p->c = currentColor;
					p++;
				}
			}				
		}

		vbObjectFA[vbObjectCount] = vbVertexCount;
		vbObjectCA[vbObjectCount] = _n;
		vbObjectCount++;

		vbVertexCount+= _n;
	}


	void Graphics::DrawQuadStripY(Vector2 _vertices[], size_t _n)
	{
		// NB_PROFILE_INCLUDE;
		if(_n < 4 || ( _n & 0x01) || _n >= vbVertexBufSize)
		{
			E_ASSERT(0);
			return;
		}

		if(vbObjectCount >= E_VB_OBJECT_BUFFER_SIZE || vbVertexCount + (int)_n >= vbVertexBufSize)
		{
			_Submit();
		}

		_SetPrimitiveType(E_TRIANGLESTRIP);
		Tex * tex = bindedTex;
		if(this->texMode && tex)
		{
			float tx0 = tex->flip_x ? 1.0f : 0.0f;
			float tx1 = tex->flip_x ? 0.0f : 1.0f;
			float ty0 = tex->flip_y ? 1.0f : 0.0f;
			float ty1 = tex->flip_y ? 0.0f : 1.0f;
			if(texMode == TM_MODULATE)
			{
				_SetVertexType(VCT::FORMAT);
				VCT * p = vbBufVCT + vbVertexCount;
				size_t k = _n >> 1;
				float dy = (ty1 - ty0)/ (k-1);
				for(size_t i = 0; i < k; i++)
				{
					int j = i * 2;
					{
						Vector2 & v = _vertices[j];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx0; p->t.y = ty0+i*dy; 
						p->c = currentColor;
						p++;
					}
					{
						Vector2 & v = _vertices[j+1];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx1; p->t.y = ty0+i*dy;
						p->c = currentColor;
						p++;
					}
				}				
			}
			else
			{
				_SetVertexType(VT::FORMAT);
				VT * p = vbBufVT + vbVertexCount;
				size_t k = _n >> 1;
				float dy = (ty1 - ty0)/ (k-1);
				for(size_t i = 0; i < k; i++)
				{
					int j = i * 2;
					{
						Vector2 & v = _vertices[j];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx0; p->t.y = ty0+i*dy; 
						//p->c = currentColor;
						p++;
					}
					{
						Vector2 & v = _vertices[j+1];
						p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
						p->t.x = tx1; p->t.y = ty0+i*dy;
					//	p->c = currentColor;
						p++;
					}
				}				
			}
		}
		else
		{
			_SetVertexType(VC::FORMAT);
			VC * p = vbBufVC + vbVertexCount;
			size_t k = _n >> 1;
		//	float dy = (1.0f - 0.0f)/ (k-1);
			for(size_t i = 0; i < k; i++)
			{
				int j = i * 2;
				{
					Vector2 & v = _vertices[j];
					p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
					//p->t.x = tx0+i*dx; p->t.y = ty0;
					p->c = currentColor;
					p++;
				}
				{
					Vector2 & v = _vertices[j+1];
					p->v.x = v.x; p->v.y = v.y; p->v.z = 0;
					//p->t.x = tx0+i*dx; p->t.y = ty1;
					p->c = currentColor;
					p++;
				}
			}				
		}

		vbObjectFA[vbObjectCount] = vbVertexCount;
		vbObjectCA[vbObjectCount] = _n;
		vbObjectCount++;

		vbVertexCount+= _n;
	}


	void Graphics::SetColor(InternalColor _c)
	{
		currentColor = _c;
		if(g_is_opengl)
		{
#	ifdef E_CFG_OPENGL
			RGBA rgba;
			rgba.SetRgbaDword(_c);
			glColor4ubv((uint8*)&_c);
#	endif
		}
	}

	void Graphics::SetColor(const RGBA & _c)
	{
		E_ASSERT(_c.IsNormal());

		currentColor = this->RGBAtoInternalColor(_c);

		if(g_is_opengl)
		{
#	ifdef E_CFG_OPENGL
			glColor4fv((float*)&_c);
#	endif
		}
	}
	

	void Graphics::SetClearColor(InternalColor _c)
	{
		clearColor = _c;

		if(g_is_opengl)
		{
#	ifdef E_CFG_OPENGL
			RGBA rgba;
			rgba.SetRgbaDword(_c);
			glClearColor(rgba.r, rgba.g, rgba.b, rgba.a);
#	endif
		}
	}

	void Graphics::SetClearColor(const RGBA & _c)
	{
		E_ASSERT(_c.IsNormal());

		clearColor = this->RGBAtoInternalColor(_c);

		if(g_is_opengl)
		{
#	ifdef E_CFG_OPENGL
			glClearColor(_c.r, _c.g, _c.b, _c.a);
#	endif
		}
	
	}

	void Graphics::SetTexMode(TexMode _mode)
	{
		// NB_PROFILE_INCLUDE;
		if(this->texMode == _mode)
		{
			return;
		}
		CheckAndCommit();

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			if(_mode)
			{
				glEnable(GL_TEXTURE_2D);
				switch(_mode)
				{
				default:
					E_ASSERT(0);
				case TM_MODULATE:
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					break;
				case TM_REPLACE:
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
					break;
				}
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			switch(_mode)
			{
			default:
				E_ASSERT(0);
			case TM_MODULATE:
				imp->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				imp->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				break;
			case TM_REPLACE:
				imp->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				imp->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				break;
			case TM_DISABLE:
				imp->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
				imp->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				break;
			}
#endif
		}

		this->texMode = _mode;
	}


//	void Graphics::SetTextureWrap(bool _wrap_x, bool _wrap_y)
//	{
//		// NB_PROFILE_INCLUDE;
//		CheckAndCommit();
//
//#ifdef E_CFG_OPENGL
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap_x ? GL_REPEAT : GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap_y ? GL_REPEAT : GL_CLAMP_TO_EDGE);
//#endif
//#ifdef E_CFG_DIRECT3D
//		imp->device->SetSamplerState(0, D3DSAMP_ADDRESSU, _wrap_x ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
//		imp->device->SetSamplerState(0, D3DSAMP_ADDRESSV, _wrap_y ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
//#endif
//	}

	void Graphics::SetBlendMode(BlendMode _bm)
	{
		if(blendMode == _bm)
		{
			return;
		}
		CheckAndCommit();

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			if(_bm)
			{
				glEnable(GL_BLEND);
				switch(_bm)
				{
				default:
				case BM_NORMAL:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case BM_ADD:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					break;
				case BM_INVERT:
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
					break;
				}
			}
			else
			{
				glDisable(GL_BLEND);
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			if(_bm)
			{
				imp->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				switch(_bm)
				{
				default:
				case BM_NORMAL:
					imp->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					imp->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					break;
				case BM_ADD:
					imp->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					imp->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					break;
				case BM_INVERT:
					imp->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
					imp->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
					break;
				}
			}
			else
			{
				imp->device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			}
#endif
		}
		blendMode = _bm;
	}

	void Graphics::ClearBuffer(bool _color, bool _z, bool _stencil)
	{
		//E_ASSERT(vbObjectCount == 0);
		//E_ASSERT(!_color || !imp->isBufferCleared); // clear more than one time after BeginScense()

		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			GLuint flags = 0;
			if(_color)
			{
				flags|= GL_COLOR_BUFFER_BIT;
			}

			if(_z)
			{
				flags|= GL_DEPTH_BUFFER_BIT;
			}

			if(_stencil)
			{
				flags|= GL_STENCIL_BUFFER_BIT;
			}

			if(flags)
			{
				if(_color)
				{
					// OpenGL color mask affects glClear()
					glDepthMask(GL_TRUE);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glClear(flags);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
					//glDepthMask(GL_FALSE);
				}
				else
				{
					glClear(flags);
				}
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			DWORD flags = 0;
			if(_color)
			{
				flags|= D3DCLEAR_TARGET;
			}
			if(_z)
			{
				flags|= D3DCLEAR_ZBUFFER;
			}
			if(_stencil)
			{
				flags|= D3DCLEAR_STENCIL;
			}
			if(flags)
			{
				D3DRECT rc = {0, 0, imp->winW, imp->winH};
				imp->device->Clear(1, &rc, flags, clearColor, 1.0f, 0);
			}
#endif
		}

#ifdef NB_DEBUG
			imp->isBufferCleared = true;
#endif
	}

	bool Graphics::TryRestoreDevice()
	{
		if(imp->isDeviceOperational)
		{
			return true;
		}
		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			HRESULT hr = imp->device->TestCooperativeLevel();
			E_ASSERT(hr != D3D_OK);
			if(hr == D3DERR_DEVICENOTRESET)
			{
				hr = imp->device->Reset(&imp->d3dpp);
				if(SUCCEEDED(hr))
				{
					_OnCreateDevice();
				}
				else
				{
					E_ASSERT(hr != D3DERR_INVALIDCALL); // there still resources to be free.
					E_ASSERT(0);
					return false;
				}
			}
			else
			{
				return false;
			}
#endif
		}
		return true;
	}

	bool Graphics::BeginScene()
	{
		// NB_PROFILE_INCLUDE;

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL

#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			if(!imp->isDeviceOperational)
			{
				if(!TryRestoreDevice())
				{
					return false;
				}
			}
		
			if(FAILED(imp->device->BeginScene()))
			{
				return false;
			}
#endif
		}

#	ifdef NB_DEBUG
			imp->isBufferCleared = false;
			imp->isRendering = true;
#	endif
			//this->ClearBuffer(false, false);
			return true;
	}

	void Graphics::EndScene()
	{
		// NB_PROFILE_EXCLUDE; 
		CheckAndCommit();
		if(this->bindedTex)
		{
			BindTex(0);
		}

#	ifdef NB_DEBUG
		imp->isRendering = false;
#	endif
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glFlush();
			if(imp->gf.doubleBuffer && !bindedFbo)
			{
	#	ifdef NB_WINDOWS
				::SwapBuffers((HDC)imp->hDC);		
	#	endif
	#	ifdef NB_LINUX
				::glXSwapBuffers(_display, imp->hWnd);
	#	endif
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			imp->device->EndScene();
			if(!bindedFbo)
			{
				HRESULT hr = imp->device->Present(NULL, NULL, NULL, NULL);
				if(hr == D3DERR_DEVICELOST)
				{
					this->_OnLostDevice();
				}
			}
#endif
		}
	}
	void Graphics::SetFont(Font * _font)
	{ 
		if(bindedFont == _font)
		{
			return; 
		}
		if(_font)
		{
			_font->AddRef();
		}
		if(bindedFont)
		{
			bindedFont->Release();
		}
		bindedFont = _font;
	}

//	void Graphics::SetFont(Font * _font)
//	{
//		// NB_PROFILE_INCLUDE;
//		if(_font == bindedFont)
//		{
//			return;
//		}
//
////		CheckAndCommit();
////
////		if(_font)
////		{
////#ifdef E_CFG_OPENGL
////			glBindTexture(GL_TEXTURE_2D, _tex->GetNative());
////#endif
////
////#ifdef E_CFG_DIRECT3D
////			imp->device->SetTexture(0, DXTEX(_tex->GetNative()));
////#endif
////		}
////		else
////		{
////#ifdef E_CFG_OPENGL
////			glBindTexture(GL_TEXTURE_2D, 0);
////#endif
////
////#ifdef E_CFG_DIRECT3D
////			imp->device->SetTexture(0, 0);
////#endif		
////		}
//
//		bindedFont = _font;
//	}
//
	//void Graphics::BindTex(uint _tex)
	//{
	//	_SetTex(imp->texTable.Get(_tex));
	//}

	void Graphics::BindTex(Tex * _tex)
	{
		// NB_PROFILE_INCLUDE;
		if(_tex == bindedTex)
		{
			return;
		}

		CheckAndCommit();

#ifdef NB_DEBUG
		imp->debugTexSwitchCount++;
#endif

		if(_tex)
		{
			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				glBindTexture(GL_TEXTURE_2D, _tex->GetNative());
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				imp->device->SetTexture(0, DXTEX(_tex->GetNative()));
#endif
			}
			_tex->AddRef();
		}
		else
		{
			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				glBindTexture(GL_TEXTURE_2D, 0);
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				imp->device->SetTexture(0, 0);
#endif		
			}
		}
		if(bindedTex)
		{
			bindedTex->Release();
		}
		bindedTex = _tex;
	}

	void Graphics::SetShader(Shader * _shader)
	{
		if(_shader == currentShader)
		{
			return;
		}

		CheckAndCommit();

		if(_shader)
		{
			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				glUseProgram(_shader->GetNative());
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				E_ASSERT(0);				
#endif
			}
			_shader->AddRef();
		}
		else
		{
			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				glUseProgram(0);				
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				E_ASSERT(0);
#endif		
			}
		}
		if(currentShader)
		{
			currentShader->Release();
		}
		currentShader = _shader;
	}

	void Graphics::SetUniform(uintx _loc, float _v)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform1f(_loc, _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const float * _v, uint _n)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform1fv(_loc, _n, _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const Vector2 & _v)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform2f(_loc, _v.x, _v.y);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const Vector2 * _v, uint _n)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform2fv(_loc, _n, (const float*) _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const Vector3 & _v)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform3f(_loc, _v.x, _v.y, _v.z);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const Vector3 * _v, uint _n)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform3fv(_loc, _n, (const float*) _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const Vector4 & _v)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform4f(_loc, _v.x, _v.y, _v.z, _v.w);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const Vector4 * _v, uint _n)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform4fv(_loc, _n, (const float*) _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}


	void Graphics::SetUniform(uintx _loc, int _v)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform1i(_loc, _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}

	void Graphics::SetUniform(uintx _loc, const int * _v, uint _n)
	{
		E_ASSERT(currentShader != 0 && vbObjectCount==0 && !needSubmitMatrix);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glUniform1iv(_loc, _n, _v);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			E_ASSERT(0);
#endif		
		}
	}


	/*bool Graphics::SetFbo(uint _id)
	{
		return SetFbo(imp->fboTable.Get(_id));
	}*/
	bool Graphics::SetFbo(Fbo * _fbo)
	{
		// NB_PROFILE_INCLUDE;
		if(!imp->isDeviceOperational && _fbo)
		{
			return false;
		}
		E_ASSERT(!imp->isRendering);
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			if(glBindFramebufferEXT == 0)
			{
				E_ASSERT(0);
				return false;
			}
#endif
		}
		if(bindedFbo == _fbo)
		{
			return true;
		}
		CheckAndCommit();

		if(bindedTex)
		{
			BindTex(0);
		}


		if(_fbo)
		{
			if(!bindedFbo)
			{
				saved_viewport_x = imp->viewX;
				saved_viewport_y = imp->viewY;
				saved_viewport_w = imp->viewW;
				saved_viewport_h = imp->viewH;
			}

			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo->GetNative());
				// E_ASSERT(glGetError() == 0);
				GLuint texID = _fbo->GetTex()->GetNative();
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT,
					GL_TEXTURE_2D,
					texID,
					0);
				// E_ASSERT(glGetError() == 0);
				GLenum fbstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
				glViewport(0, 0, _fbo->w, _fbo->h);
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				imp->device->SetRenderTarget(0, DXFBO(_fbo->GetNative()));
				_SetViewport(0, 0, _fbo->w, _fbo->h);
#endif
			}
			//E_TRACE_LINE(L"_fbo->w = " + string(_fbo->w));
		//	E_TRACE_LINE(L"_fbo->h = " + string(_fbo->h));
			//_SetViewport(0, _fbo->h, _fbo->w, _fbo->h);

			_fbo->AddRef();
		}
		else
		{

			if(g_is_opengl)
			{
#ifdef E_CFG_OPENGL
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#endif
			}
			else
			{
#ifdef E_CFG_DIRECT3D
				imp->device->SetRenderTarget(0, imp->pBackBuffer);
#endif
			}

			_SetViewport(saved_viewport_x, saved_viewport_y, saved_viewport_w, saved_viewport_h);
		}
		if(bindedFbo)
		{
			bindedFbo->Release();
		}
		bindedFbo = _fbo;

		return true;
	}
	

	FboRef Graphics::CreateFbo(int _w, int _h)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(imp->isDeviceOperational);
		if(!GetCapabilities ().frameBufferObject)
		{
			E_ASSERT(0);
			return 0;
		}
		Fbo * fbo = enew Fbo(imp, _w, _h);
		return fbo;
	}

	void Graphics::CalcOrtho2D(Matrix4 & _mat, float _left, float _right, float _bottom, float _top)
	{
//#ifdef E_CFG_OPENGL
//			glMatrixMode(GL_MODELVIEW);
//			glPushMatrix();
//			glLoadIdentity();
//			glOrtho(_left, _right, _bottom, _top, -100, 100);
//			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&_mat);
//			glPopMatrix();
//#endif
		float w = _right - _left;
		float h = _bottom - _top;

		// the top-left filling rule
		// see D3DXMatrixOrthoOffCenterRH document
		float bs = g_is_opengl ? 0 : 0.5f;
		float l = +bs;
		float r = (float)w + bs;
		float b = (float)h + bs;
		float t = +bs;
		float zn = -100.0f;
		float zf = 100.0f;
			
		_mat.LoadIdentity();
		_mat[0][0] = 2 / (float)w;
		_mat[1][1] = -2 / (float)h;
		_mat[2][2] = 2 / (zn - zf);
		_mat[3][0] = (l+r)/(l-r);
		_mat[3][1] = (b+t)/(b-t);
		_mat[3][2] = zn / (zn - zf);
	}

	void Graphics::CalcPerspective(Matrix4 & _mat, float _fovyArc, float _aspect, float _near, float _far)
	{
//
//#ifdef E_CFG_OPENGL
//			glMatrixMode(GL_MODELVIEW);
//			glPushMatrix();
//			glLoadIdentity();
//			gluPerspective(_fovyArc * 180 / PI, _aspect, _near, _far);
//			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&_mat);
//			glPopMatrix();
//#endif
		float zn = _near;
		float zf = _far;

		float yScale = 1 / tan(_fovyArc/ 2);
		float xScale = yScale / _aspect;

		_mat.LoadIdentity();
		_mat[0][0] = xScale;
		_mat[1][1] = yScale;
		_mat[2][2] = zf / (zn - zf);
		_mat[2][3] = -1;
		_mat[3][2] = zn * zf / (zn - zf);
		_mat[3][3] = 0;
	}

	void Graphics::CalcLookAt(Matrix4 & _mat, const Vector3 & _eye, const Vector3 & _lookAt, const Vector3 & _up)
	{
//#ifdef E_CFG_OPENGL
//			glMatrixMode(GL_MODELVIEW);
//			glPushMatrix();
//			glLoadIdentity();
//			gluLookAt(_eye.x, _eye.y, _eye.z, _lookAt.x, _lookAt.y, _lookAt.z, _up.x, _up.y, _up.z);
//			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&_mat);
//			glPopMatrix();
//#endif


		Vector3 z = (_eye - _lookAt).GetNormal();
		Vector3 x = (_up.Cross(z)).GetNormal();
		Vector3 y = z.Cross(x);
    
		real a[16] =
		{ 
			x.x, y.x, z.x, 0,
			x.y, y.y, z.y, 0,
			x.z, y.z, z.z, 0,
			-x.Dot(_eye), -y.Dot(_eye), -z.Dot(_eye), 1,
		};

		memcpy((float*)&_mat, a, sizeof(a));
	}

	//void Graphics::CalcLookAt1(Matrix4 & _mat, const Vector3 & _eye, const Vector3 & _lookAt, const Vector3 & _up)
	//{
	//	Vector3 z = (_lookAt - _eye).GetNormal();
	//	Vector3 x = (_up.Cross(z)).GetNormal();
	//	Vector3 y = z.Cross(x);
 //   
	//	real a[16] =
	//	{ 
	//		x.x, y.x, z.x, 0,
	//		x.y, y.y, z.y, 0,
	//		x.z, y.z, z.z, 0,
	//		-x.Dot(_eye), -y.Dot(_eye), -z.Dot(_eye), 1,
	//	};

	//	memcpy((float*)&_mat, a, sizeof(a));
	//}


	void Graphics::GetViewport(int & _x, int & _y, int & _w, int & _h) const
	{
		_x = imp->viewX;
		_y = imp->viewY;
		_w = imp->viewW;
		_h = imp->viewH;
	}

	void Graphics::_SetViewport(int _x, int _y, int _w, int _h)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glViewport(_x, imp->winH - _y - _h, _w, _h);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			D3DVIEWPORT9 vp;
			vp.X      = _x;
			vp.Y      = _y;
			vp.Width  = _w;
			vp.Height = _h;
			vp.MinZ   = 0.0f;
			vp.MaxZ   = 1.0f;
			imp->device->SetViewport(&vp);
#endif
		}
	}

	void Graphics::SetViewport(int _x, int _y, int _w, int _h)
	{
		// NB_PROFILE_INCLUDE;
		if(_w <= 0)
		{
			_w = 1;
		}
		if(_h <= 0)
		{
			_h = 1;
		}
		_SetViewport(_x, _y, _w, _h);
		imp->viewX = _x;
		imp->viewY = _y;
		imp->viewW = _w;
		imp->viewH = _h;
	}


	void Graphics::DrawString(float _x, float _y, const Char * _buf, int _len)
	{
		if(_len <= 0 || bindedFont == 0)
		{
			return;
		}

		FontImp * fi = bindedFont->imp;

		if(fi->g != imp)
		{
			fi->Attach(imp);
		}

		float x0 = _x;
		float y0 = _y;
		for(int i = 0; i < _len; i++)
		{
			Char ch = _buf[i];
			TexGlyph * glyph = fi->GetGlyph(ch);
			if(glyph == 0 || !glyph->Loaded())
			{
				glyph = fi->LoadGlyph(ch, false);
			}

			if(bindedTex != glyph->tex)
			{
				BindTex(glyph->tex);
			}

			float x1 = x0 + glyph->advance;
			float y1 = y0 + fi->lineHeight;
			DrawQuad(x0, y0, x1, y1, glyph->x0, glyph->y0, glyph->x1, glyph->y1);
			x0 = x1;
		}
	}

	void Graphics::DrawString(float _x0, float _y0, float _x1, float _y1, const Char * _buf, int _len)
	{
		if(_len <= 0 || bindedFont == 0)
		{
			return;
		}

		FontImp * fi = bindedFont->imp;

		if(fi->g != imp)
		{
			fi->Attach(imp);
		}

		float x = _x0;
		float y = _y0;
		float xx, yy;
		for(int i = 0; i < _len; i++)
		{
			Char ch = _buf[i];
			TexGlyph * glyph = fi->GetGlyph(ch);
			if(glyph == 0 || !glyph->Loaded())
			{
				glyph = fi->LoadGlyph(ch, false);
			}

			if(bindedTex != glyph->tex)
			{
				BindTex(glyph->tex);
			}

_AGAIN:
			xx = x + glyph->advance;
			yy = y + fi->lineHeight;
			if(xx > _x1)
			{
				if(yy > _y1)
				{
					return;
				}
				x = _x0;
				y = yy;
				goto _AGAIN;
			}
			DrawQuad(x, y, xx, yy, glyph->x0, glyph->y0, glyph->x1, glyph->y1);
			x = xx;
		}
	}

	//int Graphics::MajorVersion() const
	//{
	//	return imp->majorVersion;
	//}

	//int Graphics::MinorVersion() const
	//{
	//	return imp->minorVersion;
	//}

	void Graphics::SetFogParam(float _r, float _g, float _b, float _density, float _start, float _end, int _mode)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			float c[] = {_r, _g, _b, 1.0f};
			glFogfv(GL_FOG_COLOR, c);
			glFogf(GL_FOG_DENSITY, _density);
			glHint(GL_FOG_HINT, GL_DONT_CARE);
			glFogf(GL_FOG_START, _start);
			glFogf(GL_FOG_END, _end);
			glFogi(GL_FOG_MODE, GL_LINEAR);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			DWORD fogColor = ((int(_r * 255)  & 0x000000ff)<<16) |  ((int(_g * 255)  & 0x000000ff)<<8)|  ((int(_b * 255)  & 0x000000ff)<<0);
			imp->device->SetRenderState(D3DRS_FOGCOLOR, fogColor);
			//if(D3DFOG_LINEAR == Mode)
			{
				imp->device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
				imp->device->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&_start));
				imp->device->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&_end));
			}
#endif	
		}
	}

	void Graphics::SetMaterial(Material & _material)
	{
		CheckAndCommit();

#ifdef NB_DEBUG
		imp->debugMaterialSwitchCount++;
#endif

		if(!_material.tex && !_material.tex_name.empty())
		{
			_material.tex = LoadTexFromPool(_material.tex_name);
		}

		if(_material.tex)
		{
			SetTexMode(TM_MODULATE);
			BindTex(_material.tex);
		}
		else
		{
			//_g->SetColor(m->diffuse);
			SetColor(_material.diffuse);
			SetTexMode(TM_DISABLE);
			BindTex(0);
		}


		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glMaterialfv(GL_FRONT, GL_AMBIENT, &_material.ambient.r);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, &_material.diffuse.r);
			if(_material.render_mode == 2)
			{
				glMaterialfv(GL_FRONT, GL_SPECULAR, &_material.specular.r);
				glMaterialf(GL_FRONT, GL_SHININESS, _material.specular_power);
			}
			else
			{
				float tmp[4] = {0, 0, 0, 0};
				glMaterialfv(GL_FRONT, GL_SPECULAR, tmp);
			}
			glMaterialfv(GL_FRONT, GL_EMISSION, &_material.emissive.r);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			imp->device->SetRenderState(D3DRS_SPECULARENABLE, _material.render_mode == 2 ? TRUE : FALSE);
			E_ASSERT(sizeof(D3DMATERIAL9) < sizeof(Material));
			D3DMATERIAL9 m = *((const D3DMATERIAL9*) &_material);
			//m.Power*= 0.32f;
			imp->device->SetMaterial(&m);
#endif	
		}
	}

	void Graphics::SetAmbient(const RGBA & _ambient)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &_ambient.r);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			imp->device->SetRenderState(D3DRS_AMBIENT, RGBAtoInternalColor(_ambient));
#endif	
		}
	}


	void Graphics::SetLight(int _index, const Light * _light)
	{
		E_ASSERT(_index >= 0 && _index <= 7);
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			if(_light)
			{
				int light_index = _index + GL_LIGHT0;
				glLightfv(light_index, GL_AMBIENT, &_light->ambient.r);
				glLightfv(light_index, GL_DIFFUSE, &_light->diffuse.r);
				glLightfv(light_index, GL_SPECULAR, &_light->specular.r);
				glLightf(light_index, GL_CONSTANT_ATTENUATION, _light->attenuation.constant);
				glLightf(light_index, GL_LINEAR_ATTENUATION, _light->attenuation.linear);
				glLightf(light_index, GL_QUADRATIC_ATTENUATION, _light->attenuation.quadratic);
				switch(_light->type)
				{
				case 1:
					// point
					{
						Vector4 pos = {_light->position.x,_light->position.y, _light->position.z, 1.0f} ;
						glLightfv(light_index, GL_POSITION, &pos.x);
					}
					break;
				case 3:
					// directional
					{
						Vector4 pos = {-_light->direction.x, -_light->direction.y, -_light->direction.z, 0.0f} ;
						glLightfv(light_index, GL_POSITION, &pos.x);
					}
					break;
				case 2:
				default:
					E_ASSERT(0);
					break;
				}
				glEnable(GL_LIGHT0 + _index);
			}
			else
			{
				glDisable(GL_LIGHT0 + _index);
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			if(_light)
			{
				D3DLIGHT9 tmp;
				memcpy(&tmp, _light, sizeof(Light));
				tmp.Attenuation0 = _light->attenuation.constant;
				tmp.Attenuation1 = _light->attenuation.linear;
				tmp.Attenuation2 = _light->attenuation.quadratic;
				tmp.Range = 1.0e10f;
				tmp.Falloff = 1;
				tmp.Theta = 0.15f * PI;
				tmp.Phi = 0.25f * PI;
				imp->device->SetLight(_index, &tmp);
				imp->device->LightEnable(_index, TRUE);
			}
			else
			{
				imp->device->LightEnable(_index, FALSE);
			}
#endif	
		}
	}

//	void Graphics::EnableLight(int _index, bool _enable)
//	{
//		E_ASSERT(_index >= 0 && _index <= 7);
//		CheckAndCommit();
//		if(g_is_opengl)
//		{
//#ifdef E_CFG_OPENGL
//			if(_enable)
//			{
//				glEnable(GL_LIGHT0 + _index);
//			}
//			else
//			{
//				glDisable(GL_LIGHT0 + _index);
//			}
//#endif
//		}
//		else
//		{
//#ifdef E_CFG_DIRECT3D
//			imp->device->LightEnable(_index, _enable);
//#endif	
//		}
//	}

//	void Graphics::SetWireFrameMode(bool _wired)
//	{
//		CheckAndCommit();
//		render_state.wire_frame_mode = _wired;
//		if(g_is_opengl)
//		{
//#ifdef E_CFG_OPENGL
//			if(_wired)
//			{
//				glPolygonMode(GL_FRONT_AND_BACK ,GL_LINE);
//			}
//			else
//			{
//				glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
//			}
//#endif
//		}
//		else
//		{
//#ifdef E_CFG_DIRECT3D
//			imp->device->SetRenderState(D3DRS_FILLMODE, _wired ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
//#endif	
//		}
//	}

	//bool Graphics::GetWireFrameMode() const
	//{
	//	return wire_frame_mode;
	//}
	
	void Graphics::_SetVertexType(int _newVertexType)
	{
		if(_newVertexType != this->vbVertexType)
		{
			CheckAndCommit();
			// E_ASSERT(_newVertexType < _VT_MASK+1);
			vbVertexType = _newVertexType;
			vbStride = g_vtStrideTable[vbVertexType];
			vbVertexBufSize = E_VB_VERTEX_BUFFER_SIZE / vbStride;
		}
	}

	void Graphics::_SetPrimitiveType(Primitive _newPrimitive)
	{
		if(_newPrimitive != vbPrimitive)
		{
			CheckAndCommit();
			vbPrimitive = _newPrimitive;
		}
	}

	void Graphics::SetProjectViewMatrix(const Matrix4 & _project, const Matrix4 & _view)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf((float*)&_project);
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf((float*)&_view);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			imp->project_matrix = _project;
			imp->view_matrix = _view;
			imp->device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&_project);
			imp->device->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&_view);
			Matrix4 m;
			m.LoadIdentity();
			SetMatrix(m);
#endif
		}
	}

	void Graphics::_Submit()
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(imp->isRendering || vbObjectCount == 0);

		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			if(needSubmitMatrix)
			{
				needSubmitMatrix = false;
				imp->device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&imp->model_matrix_stack.top());
			}
#endif
		}

		if(vbObjectCount && imp->SetVertexSource(vbBuf, vbStride, vbVertexType, vbVertexCount))
		{
			E_ASSERT(vbObjectCount <= E_VB_OBJECT_BUFFER_SIZE);
			imp->DrawMultiPrimitive(vbPrimitive, vbObjectFA, vbObjectCA, vbObjectCount);
		}
		vbObjectCount = 0;
		vbVertexCount = 0;
	}


//	void Graphics::SetActiveMatrixStack(MatrixStackID _mt)
//	{
//		if(currentMatrixStack == _mt)
//		{
//			return;
//		}
//		CheckAndCommit();
//
//
//		if(g_is_opengl)
//		{
//#ifdef E_CFG_OPENGL
//			glMatrixMode(_mt == MT_MODELVIEW ? GL_MODELVIEW : GL_PROJECTION);
//#endif
//		}
//		else
//		{
//#ifdef E_CFG_DIRECT3D
//			if(needSubmitMatrix)
//			{
//				if(currentMatrixStack == MT_MODELVIEW)
//				{
//					imp->device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&imp->model_matrix_stack.top());
//				}
//				else
//				{
//					imp->device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&imp->model_matrix_stack.top());
//				}
//
//				needSubmitMatrix = false;
//			}
//#endif
//		}
//		currentMatrixStack = _mt;
//	}

	void Graphics::PushMatrix()
	{
		// NB_PROFILE_INCLUDE;
		//E_ASSERT(currentMatrixStack == MT_MODELVIEW);
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glPushMatrix();
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			imp->model_matrix_stack.push(imp->model_matrix_stack.top());
#endif
		}
	}

	void Graphics::PopMatrix()
	{
		// NB_PROFILE_INCLUDE;
		CheckAndCommit();

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glPopMatrix();
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			needSubmitMatrix = false;
			imp->model_matrix_stack.pop();
			imp->device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&imp->model_matrix_stack.top());
#endif
		}
	}

	void Graphics::SetMatrix(const Matrix4 & _mat)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glLoadMatrixf((float*)&_mat);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			needSubmitMatrix = true;
			imp->model_matrix_stack.top() = _mat;
#endif
		}
	}

	void Graphics::GetMatrix(Matrix4 & _mat) const
	{
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&_mat);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			//ID3DXMatrixStack * ms = imp->model_matrix_stack;
			_mat = imp->model_matrix_stack.top();
#endif
		}
	}

	void Graphics::TranslateMatrix(float _x, float _y, float _z)
	{
		// NB_PROFILE_INCLUDE;
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glTranslatef(_x, _y, _z);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			needSubmitMatrix = true;
			//ID3DXMatrixStack * ms = imp->model_matrix_stack;
			//ms->TranslateLocal(_x, _y, _z);
			imp->model_matrix_stack.top().Translate(_x, _y, _z);
#endif
		}
	}

	void Graphics::RotateMatrix(float _angle, float _x, float _y, float _z)
	{
		// NB_PROFILE_INCLUDE;
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glRotatef(_angle * 180 /  PI, _x, _y, _z);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			needSubmitMatrix = true;
		//	ID3DXMatrixStack * ms = imp->model_matrix_stack;
		//	ms->RotateAxisLocal(&D3DXVECTOR3(_x, _y, _z), _angle);
			imp->model_matrix_stack.top().Rotate(_angle, _x, _y, _z);
#endif
		}
	}

	void Graphics::ScaleMatrix(float _x, float _y, float _z)
	{
		// NB_PROFILE_INCLUDE;
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glScalef(_x, _y, _z);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			needSubmitMatrix = true;
			//ID3DXMatrixStack * ms = imp->model_matrix_stack;
			imp->model_matrix_stack.top().Scale(_x, _y, _z);
#endif
		}
	}

	void Graphics::MultMatrix(Matrix4 & _mat)
	{
		// NB_PROFILE_INCLUDE;
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			glMultMatrixf((float*)&_mat);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			needSubmitMatrix = true;
			//ID3DXMatrixStack * ms = imp->model_matrix_stack;
			imp->model_matrix_stack.top().Multiply(_mat);
#endif
		}
	}

	bool Graphics::IsDeviceOperational() const
	{ 
		return imp->isDeviceOperational;
	}

//	void Graphics::SetLoadTexFunc(E_LOAD_TEX_FUNC _func, void * _param)
//	{
//		imp->loadTexFunc = _func;
//		imp->loadTexFuncParam = _param;
//	}
	
	bool Graphics::SetVertexSource(void * _p, uint _stride, int _format, int _vertexCount)
	{
		CheckAndCommit();
		return imp->SetVertexSource(_p, _stride, _format, _vertexCount);
	}

	void Graphics::DrawPrimitive(Primitive _type, int _firstVertex, int _vertexCount)
	{
		CheckAndCommit();
		imp->DrawPrimitive(_type, _firstVertex, _vertexCount);
	}

	void Graphics::DrawMultiPrimitive(Primitive _type, int _firstVertices[], int _vertexCounts[], int _objectCount)
	{
		CheckAndCommit();
		imp->DrawMultiPrimitive(_type, _firstVertices, _vertexCounts, _objectCount);
	}

#ifdef NB_DEBUG
	uint64 Graphics::DebugRenderOjbectCount(bool _reset)
	{
		uint64  ret = imp->debugRenderOjbectCount;
		if(_reset)
		{
			 imp->debugRenderOjbectCount = 0;
		}
		return ret;
	}

	uint64 Graphics::DebugRenderVertexCount(bool _reset)
	{
		uint64  ret = imp->debugRenderVertexCount;
		if(_reset)
		{
			 imp->debugRenderVertexCount = 0;
		}
		return ret;
	}

	uint64 Graphics::DebugTexSwitchCount(bool _reset)
	{
		uint64  ret = imp->debugTexSwitchCount;
		if(_reset)
		{
			 imp->debugTexSwitchCount = 0;
		}
		return ret;
	}

	uint64 Graphics::DebugMaterialSwitchCount(bool _reset)
	{
		uint64  ret = imp->debugMaterialSwitchCount;
		if(_reset)
		{
			 imp->debugMaterialSwitchCount = 0;
		}
		return ret;
	}
#endif

	InternalColor Graphics::RGBAtoInternalColor(const RGBA & _r)
	{
		return g_is_opengl ? _r.GetRgbaDword() : _r.GetBgraDword();
	}

	RGBA Graphics::InternalColortoRGBA(InternalColor _c)
	{
		RGBA ret;
		if(g_is_opengl)
		{
			ret.SetRgbaDword(_c);
		}
		else
		{
			ret.SetBgraDword(_c);
		}
		return ret;
	}

	bool Graphics::GetImage(Image & _pic, int _x, int _y, uint _w, uint _h)
	{
		CheckAndCommit();
		_pic.Delete();

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			_pic.Alloc(_w, _h);
			//glWindowPos2i();
			//GLint pos[4] = {0, 0};
			//glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);
			glReadPixels(_x, imp->winH - _y - _pic.h, _w, _h, GL_RGBA, GL_UNSIGNED_BYTE, _pic.data);
			_pic.FlipV();
			return true;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			IDirect3DSurface9 * pSurface = 0;
			imp->device->GetRenderTarget(0, &pSurface);
			if(pSurface == NULL || pSurface != imp->pBackBuffer)
			{
				return false;
			}

			RECT rect;
			rect.left = _x;
			rect.top = _y;
			rect.right = _w + _x - 1;
			rect.bottom = _h + _y - 1;

			D3DLOCKED_RECT lockedRect;
			if(SUCCEEDED(pSurface->LockRect(&lockedRect, &rect, 0)))
			{
				_pic.Alloc(_w, _h);
				uint8 * p = (uint8 *)lockedRect.pBits;
				for(uint j = 0; j < _pic.h; j++)
				{
					uint8 * p1 = _pic.Get(0, j);
					memcpy(p1, p, _pic.w * 4);
					p+= lockedRect.Pitch;
				}

				pSurface->UnlockRect();
				pSurface->Release();
				_pic.SwapChannel(0, 2);
				return true;
			}
			else
			{
				pSurface->Release();
				return false;
			}
#endif
		}
	}

	bool Graphics::PutImage(Image & _pic, int _x, int _y)
	{
		CheckAndCommit();

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
#	ifdef NB_WINDOWS
			if(glWindowPos2i == 0)
			{
				return false;
			}
#	endif
		//glFlush();
		//glDrawBuffer(GL_FRONT_AND_BACK);
			glDisable(GL_TEXTURE_2D);
			glWindowPos2i(_x, imp->winH - _y - _pic.h);
			glDrawPixels(_pic.w, _pic.h, GL_RGBA, GL_UNSIGNED_BYTE, _pic.data);
			glRasterPos2i(0, 0);
			if(texMode)
			{
				glEnable(GL_TEXTURE_2D);
			}
			return true;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			IDirect3DSurface9 * pSurface = 0;
			imp->device->GetRenderTarget(0, &pSurface);
			if(pSurface == NULL)
			{
				return false;
			}

			RECT rect;
			rect.left = _x;
			rect.top = _y;
			rect.right = _pic.w + _x - 1;
			rect.bottom = _pic.h + _y - 1;

			D3DLOCKED_RECT lockedRect;
			if(SUCCEEDED(pSurface->LockRect(&lockedRect, &rect, 0)))
			{
				_pic.SwapChannel(0, 2);
				uint8 * p = (uint8 *)lockedRect.pBits;
				for(uint j = 0; j < _pic.h; j++)
				{
					uint8 * p1 = _pic.Get(0, j);
					memcpy(p, p1, _pic.w * 4);
					p+= lockedRect.Pitch;
				}
				pSurface->UnlockRect();
				pSurface->Release();
				_pic.SwapChannel(0, 2);
				return true;
			}
			else
			{
				pSurface->Release();
				return false;
			}
#endif
		}
	}

#ifdef E_CFG_OPENGL
	static GLuint LoadOpenGLShader(const Path & _path, GLenum _shaderType)
	{

		//const char * src[1];
		//src[0] = (const char*)mb.data;
		Array<const char *> src;
		{
			FileRef file = FS::OpenFile(_path);
			if(!file)
			{
				E_ASSERT(0);
				return 0;
			}
			stringa line;
			while(file->ReadLine(line))
			{
				int len = line.length();
				char * p = enew char[len+1];
				memcpy(p, line.c_str(), len+1);
				src.push_back(p);
			}
		}

		GLuint sid = glCreateShader(_shaderType);
		if(sid == 0)
		{
			E_ASSERT(0);
			return 0;
		}

		{
			glShaderSource(sid, src.size(), &src[0], NULL);
			for(size_t i=0; i<src.size(); i++)
			{
				delete[] src[i];
			}
		}

		glCompileShader(sid);
		GLint status;
		glGetShaderiv(sid, GL_COMPILE_STATUS, &status);
		GLsizei len;
		if(!status)
		{
			glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &len);
			stringa s;
			s.reserve(len);
			glGetShaderInfoLog(sid, len, &len, s.c_str());
			s[len] = 0;
			glDeleteShader(sid);
			message(L"[nb] (WW) " + s);
			//E_ASSERT(0);
			return 0;
		}
		return sid;	
	}
#endif
	
	Shader * Graphics::LoadShader(const Path & _vertex, const Path & _pixel)
	{
		if(!_vertex.IsValid() && !_pixel.IsValid())
		{
			message(L"[nb] (WW) !_vertex.IsValid() && !_pixel.IsValid()");
			return 0;
		}
		
		if(_vertex.IsValid() && !GetCapabilities().vertexShader)
		{
			message(L"[nb] (WW) Vertex shader is unsupported.");
			return 0;
		}

		if(_pixel.IsValid() && !GetCapabilities().pixelShader)
		{
			message(L"[nb] (WW) Pixel shader is unsupported.");
			return 0;
		}

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			GLuint pixel_shader_id = 0;
			GLuint vertex_shader_id = 0;
			GLuint pid = 0;
			
			pid = glCreateProgram();
			if(pid == 0)
			{
				message(L"[nb] (WW) Failed to create shader program.");
				goto _GLSHADER_ERR;
			}
			
			if(_vertex.IsValid())
			{
				vertex_shader_id = LoadOpenGLShader(_vertex, GL_VERTEX_SHADER);
				if(vertex_shader_id == 0)
				{
					message(L"[nb] (WW) Failed to load shader: " + _vertex.GetString());
					goto _GLSHADER_ERR;
				}
				glAttachShader(pid, vertex_shader_id);
			}

			if(_pixel.IsValid())
			{
				pixel_shader_id = LoadOpenGLShader(_pixel, GL_FRAGMENT_SHADER);
				if(pixel_shader_id == 0)
				{
					message(L"[nb] (WW) Failed to load shader: " + _pixel.GetString());
					goto _GLSHADER_ERR;
				}
				glAttachShader(pid, pixel_shader_id);
			}
				
			glLinkProgram(pid);

			{
				GLint status;
				glGetProgramiv(pid, GL_LINK_STATUS, &status);
				if(!status)
				{
					GLint len;
					glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &len);
					stringa s;
					s.reserve(len);
					glGetProgramInfoLog(pid, len, &len, s.c_str());
					s[len] = 0;
					message(L"[nb] (WW) " + s);
					goto _GLSHADER_ERR;
				}
			}

			{
				Shader * ret = enew Shader();
				ret->native = pid;
				ret->g = this;
				if(vertex_shader_id)
				{
					glDeleteShader(vertex_shader_id);				
				}
				if(pixel_shader_id)
				{
					glDeleteShader(pixel_shader_id);				
				}
				return ret;
			}

_GLSHADER_ERR:
			if(pid)
			{
				glDeleteProgram(pid);
			}
			if(vertex_shader_id)
			{
				glDeleteShader(vertex_shader_id);				
			}
			if(pixel_shader_id)
			{
				glDeleteShader(pixel_shader_id);				
			}
			return 0;
#endif
		}
		return 0;
	}

	void Graphics::SetErrorCallback(const Callback & _callback)
	{
		imp->errorCallback = _callback;
	}
	
#ifdef E_CFG_LUA

	static int LuaDrawQuad(lua_State * L)
	{
		E_ASSERT(lua_gettop(L) == 4);
		float x0 = lua_tofloat(L, 1);
		float y0 = lua_tofloat(L, 2);
		float x1 = lua_tofloat(L, 3);
		float y1 = lua_tofloat(L, 4);
		Graphics::Singleton()->DrawQuad(x0, y0, x1, y1);
		return 0;
	}

	static int LuaDrawQuad1(lua_State * L)
	{
		E_ASSERT(lua_gettop(L) == 8);
		float x0 = lua_tofloat(L, 1);
		float y0 = lua_tofloat(L, 2);
		float x1 = lua_tofloat(L, 3);
		float y1 = lua_tofloat(L, 4);
		float x2 = lua_tofloat(L, 5);
		float y2 = lua_tofloat(L, 6);
		float x3 = lua_tofloat(L, 7);
		float y3 = lua_tofloat(L, 8);
		Graphics::Singleton()->DrawQuad(
			x0, y0,  0,   0, 0,
			x1, y1,  0,   0, 1,
			x2, y2,  0,   1, 1,
			x3, y3,  0,   1, 0
			);
		return 0;
	}

	static int LuaDrawString(lua_State * L)
	{
		E_ASSERT(lua_gettop(L) == 3);
		float x0 = lua_tofloat(L, 1);
		float y0 = lua_tofloat(L, 2);
		const char * s = lua_tostring(L, 3);
		float y1 = lua_tofloat(L, 4);
		Graphics::Singleton()->DrawString(x0, y0, s);
		return 0;
	}


	static int LuaSetColor(lua_State * L)
	{
		E_ASSERT(lua_gettop(L) == 4);
		float r = lua_tofloat(L, 1);
		float g = lua_tofloat(L, 2);
		float b = lua_tofloat(L, 3);
		float a = lua_tofloat(L, 4);
		Graphics::Singleton()->SetColor(r, g, b, a);
		return 0;
	}

	bool Graphics::Register(lua_State * L)
	{
		if(!Tex::Register(L))
		{
			return false;
		}
		if(!Font::Register(L))
		{
			return false;
		}

		lua_register(L, "DrawQuad", &LuaDrawQuad);
		lua_register(L, "DrawQuad1", &LuaDrawQuad1);
		lua_register(L, "DrawString", &LuaDrawString);
		lua_register(L, "SetColor", &LuaSetColor);
		return true;
	}
#endif

	string Graphics::GetHardwareString()
	{
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			const char * p = (const char*)glGetString(GL_RENDERER);
			return p ? p : "Unkown Device";
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			D3DADAPTER_IDENTIFIER9 id;
			HRESULT hr =imp->d3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &id);
			return SUCCEEDED(hr) ? string(id.Description) : L"Unkown Device";
#endif
		}
	}

	string Graphics::GetSoftwareString()
	{
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			const char * version = (const char*)glGetString(GL_VERSION);
			string s_version(version);
			// trim addition info, such as "xxx compatible mode"
			int n = s_version.find_any(L" ");
			if(n != -1)
			{
				s_version[n] = 0;
			}
			return L"OpenGL " + s_version;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			HKEY hKEY = NULL;
			// read Windows Register
			string version = "Unkown Version";
			if(ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\DirectX\\" , 0, KEY_READ, &hKEY))
			{
				unsigned char buf[100];
				DWORD type = REG_SZ;
				DWORD size = 100;
				if(ERROR_SUCCESS == ::RegQueryValueEx(hKEY, L"Version", NULL, &type, buf, &size))
				{
					version = (Char*)buf;
				}
				::RegCloseKey(hKEY);
			}
			return L"DirectX " + version;
#endif
		}

	}

	Win * Graphics::GetWin()
	{ return imp->win; }

//
//			GS_LIGHT,
//		GS_DEPTH_TEST,
//		GS_CULL_FACE,
//	void Graphics::EnableLight(bool _enable)
//	{
//		CheckAndCommit();
//		if(g_is_opengl)
//		{
//#ifdef E_CFG_OPENGL
//			if(_enable)
//			{
//				glEnable(GL_LIGHTING);
//			}
//			else
//			{
//				glDisable(GL_LIGHTING);
//			}
//#endif
//		}
//		else
//		{
//#ifdef E_CFG_DIRECT3D
//			imp->device->SetRenderState(D3DRS_LIGHTING, _enable ? TRUE : FALSE);
//#endif	
//		}
//	}
	/*
	void Graphics::SetWireFrameMode(bool _wired)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			if(_wired)
			{
			}
			else
			{
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
#endif	
		}
	}

	*/

	void Graphics::Enable(GraphicsState _s, bool _enable)
	{
		CheckAndCommit();

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			switch(_s)
			{
			case GS_FOG:
				if(_enable)
				{
					glEnable(GL_FOG);
				}
				else
				{
					glDisable(GL_FOG);
				}
				break;
			case GS_BLEND:
				if(_enable)
				{
					glEnable(GL_BLEND);
				}
				else
				{
					glDisable(GL_BLEND);
				}
				break;
			case GS_SCISSOR_TEST:
				if(_enable)
				{
					glEnable(GL_SCISSOR_TEST);
				}
				else
				{
					glDisable(GL_SCISSOR_TEST);
				}
				break;
			case GS_LIGHTING:
				if(_enable)
				{
					glEnable(GL_LIGHTING);
				}
				else
				{
					glDisable(GL_LIGHTING);
				}				
				render_state.lighting = _enable;
				break;
			case GS_DEPTH_TEST:
				if(_enable)
				{
					glEnable(GL_DEPTH_TEST);
				}
				else
				{
					glDisable(GL_DEPTH_TEST);
				}
				break;
			case GS_CULL_FACE:
				if(_enable)
				{
					glEnable(GL_CULL_FACE);
				}
				else
				{
					glDisable(GL_CULL_FACE);
				}
				break;
			case GS_WIRE_FRAME:
				if(_enable)
				{
					glPolygonMode(GL_FRONT_AND_BACK ,GL_LINE);
				}
				else
				{
					glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
				}				
				render_state.wire_frame_mode = _enable;
				break;
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			switch(_s)
			{
			case GS_FOG:
				imp->device->SetRenderState(D3DRS_FOGENABLE, (BOOL)_enable);
				break;
			case GS_BLEND:
				imp->device->SetRenderState(D3DRS_ALPHABLENDENABLE, (BOOL)_enable);
				break;
			case GS_SCISSOR_TEST:
				imp->device->SetRenderState(D3DRS_SCISSORTESTENABLE, (BOOL)_enable);
				break;
			case GS_LIGHTING:
				imp->device->SetRenderState(D3DRS_LIGHTING, (BOOL)_enable);
				render_state.lighting = _enable;
				break;
			case GS_DEPTH_TEST:
				imp->device->SetRenderState(D3DRS_ZENABLE, (BOOL)_enable);
				break;
			case GS_CULL_FACE:
				imp->device->SetRenderState(D3DRS_CULLMODE, _enable ? D3DCULL_CW : D3DCULL_NONE);
				break;
			case GS_WIRE_FRAME:
				imp->device->SetRenderState(D3DRS_FILLMODE, _enable? D3DFILL_WIREFRAME : D3DFILL_SOLID);
				render_state.wire_frame_mode = _enable;
				break;
			}
#endif
		}
	}



	void Graphics::SetScissor(int _x0, int _y0, int _x1, int _y1)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			int yy0 = imp->winH - _y1 - 1;
			int yy1 = imp->winH - _y0 - 1;
			glScissor(_x0, yy0, _x1 - _x0 + 1, yy1 - yy0 + 1);
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			RECT rect = {_x0, _y0, _x1, _y1};
			imp->device->SetScissorRect(&rect);
#endif		
		}
	}

	void Graphics::GetScissor(int &_x0, int &_y0, int &_x1, int &_y1)
	{
		CheckAndCommit();
		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			int yy0, yy1;
			int v[4];
			glGetIntegerv(GL_SCISSOR_BOX, v);
			_x0 = v[0];
			yy0 = v[1];
			_x1 = _x0 + v[2] - 1;
			yy1 = yy0 + v[3] - 1;
			_y0 = imp->winH - yy1 - 1;
			_y1 = imp->winH - yy0 - 1;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			RECT rect = {_x0, _y0, _x1, _y1};
			imp->device->GetScissorRect(&rect);
			_x0 = rect.left;
			_y0 = rect.top;
			_x1 = rect.right;
			_y1 = rect.bottom;
#endif		
		}
	}

	void Graphics::DrawEllipse(float _xc, float _yc, float _rotate, float _a, float _b, const RGBA & _color, bool _fill, uint _n)
	{
		E_ASSERT(_n > 2);
		VC * buf = enew VC[_n + 1];
		InternalColor c = this->RGBAtoInternalColor(_color);
		float da = 2 * PI / _n;
		float angle = 0;
		for(int i=0; i<_n; i++, angle+=da)
		{
			buf[i].v.x = _a * cos(angle);
			buf[i].v.y = _b * sin(angle);
			buf[i].v.z = 0;
			buf[i].c = c;
		}
		buf[_n] = buf[0];
		_n++;

		PushMatrix();
		TranslateMatrix(_xc, _yc, 0);
		RotateMatrix(_rotate, 0, 0, 1);
		SetVertexSource(buf, sizeof(VC), VC::FORMAT, _n);
		SetTexMode(TM_DISABLE);
		DrawPrimitive(_fill ? E_TRIANGLEFAN : E_LINESTRIP, 0, _n);
		SetTexMode(TM_MODULATE);
		PopMatrix();
		delete[] buf;
	}

	void Graphics::DrawQuad(float _xc, float _yc, float _rotate, float a, float b, const RGBA & _color, bool _fill)
	{
		InternalColor c = this->RGBAtoInternalColor(_color);
		VC pt[5];
		pt[0].v.x = -a;
		pt[0].v.y = -b;
		pt[0].v.z = 0;
		pt[0].c = c;

		pt[1].v.x = a;
		pt[1].v.y = -b;
		pt[1].v.z = 0;
		pt[1].c = c;

		pt[2].v.x = a;
		pt[2].v.y = b;
		pt[2].v.z = 0;
		pt[2].c = c;

		pt[3].v.x = -a;
		pt[3].v.y = b;
		pt[3].v.z = 0;
		pt[3].c = c;

		pt[4].v.x = -a;
		pt[4].v.y = -b;
		pt[4].v.z = 0;
		pt[4].c = c;

		int n = 5;
		PushMatrix();
		TranslateMatrix(_xc, _yc, 0);
		RotateMatrix(_rotate, 0, 0, 1);
		SetVertexSource(pt, sizeof(VC), VC::FORMAT, n);
		SetTexMode(TM_DISABLE);
		DrawPrimitive(_fill ? E_TRIANGLEFAN : E_LINESTRIP, 0, n);
		SetTexMode(TM_MODULATE);
		PopMatrix();
	}

	void Graphics::DrawTriangle(float _xc, float _yc, float _rotate, float a, float b, const RGBA & _color, bool _fill)
	{
		InternalColor c = this->RGBAtoInternalColor(_color);
		VC pt[5];
		pt[0].v.x = 0;
		pt[0].v.y = -b;
		pt[0].v.z = 0;
		pt[0].c = c;

		pt[1].v.x = a;
		pt[1].v.y = b;
		pt[1].v.z = 0;
		pt[1].c = c;

		pt[2].v.x = -a;
		pt[2].v.y = b;
		pt[2].v.z = 0;
		pt[2].c = c;

		pt[3].v.x = 0;
		pt[3].v.y = -b;
		pt[3].v.z = 0;
		pt[3].c = c;

		int n = 4;
		PushMatrix();
		TranslateMatrix(_xc, _yc, 0);
		RotateMatrix(_rotate, 0, 0, 1);
		SetVertexSource(pt, sizeof(VC), VC::FORMAT, n);
		SetTexMode(TM_DISABLE);
		DrawPrimitive(_fill ? E_TRIANGLEFAN : E_LINESTRIP, 0, n);
		SetTexMode(TM_MODULATE);
		PopMatrix();

	}

	//void Graphics::DrawLine(float _x0, float _y0, const RGBA & _color0, float _x1, float _y1, const RGBA & _color1)
	//{
	//	VC pt[2];
	//	pt[0].v.x = _x0;
	//	pt[0].v.y = _y0;
	//	pt[0].v.z = 0;
	//	pt[0].c = RGBAtoInternalColor(_color0);

	//	pt[1].v.x = _x1;
	//	pt[1].v.y = _y1;
	//	pt[1].v.z = 0;
	//	pt[1].c = RGBAtoInternalColor(_color1);

	//	SetTexMode(TM_DISABLE);
	//	SetVertexSource(pt, sizeof(VC), VC::FORMAT, 2);
	//	DrawPrimitive(E_LINESTRIP, 0, 2);
	//	SetTexMode(TM_MODULATE);

	//}

	void Graphics::DrawLine(float _x0, float _y0, float _z0, const RGBA & _color0, float _x1, float _y1, float _z1, const RGBA & _color1)
	{
		VC pt[2];
		pt[0].v.x = _x0;
		pt[0].v.y = _y0;
		pt[0].v.z = _z0;
		pt[0].c = RGBAtoInternalColor(_color0);

		pt[1].v.x = _x1;
		pt[1].v.y = _y1;
		pt[1].v.z = _z1;
		pt[1].c = RGBAtoInternalColor(_color1);

		SetTexMode(TM_DISABLE);
		SetVertexSource(pt, sizeof(VC), VC::FORMAT, 2);
		DrawPrimitive(E_LINESTRIP, 0, 2);
		SetTexMode(TM_MODULATE);

	}

	void Graphics::DrawHalfAxisGrid(float _size)
	{
		RGBA xc0 = {1.0f, 0, 0, 1};
		RGBA yc0 = {0, 1.0f, 0, 1};
		RGBA zc0 = {0, 0, 1.0f, 1};

		InternalColor xc = RGBAtoInternalColor(xc0);
		InternalColor yc = RGBAtoInternalColor(yc0);
		InternalColor zc = RGBAtoInternalColor(zc0);

		VC pt[6][10][2];
		float delta = _size / 10;
		for(int i=0; i<10; i++)
		{
			pt[0][i][0].v.x = 0;
			pt[0][i][0].v.y = i*delta;
			pt[0][i][0].v.z = 0;
			pt[0][i][0].c = xc;
			pt[0][i][1] = pt[0][i][0];
			pt[0][i][1].v.x = _size;

			pt[1][i][0].v.x = 0;
			pt[1][i][0].v.y = 0;
			pt[1][i][0].v.z = i*delta;
			pt[1][i][0].c = xc;
			pt[1][i][1] = pt[1][i][0];
			pt[1][i][1].v.x = _size;
			
			pt[2][i][0].v.x = i*delta;
			pt[2][i][0].v.y = 0;
			pt[2][i][0].v.z = 0;
			pt[2][i][0].c = yc;
			pt[2][i][1] = pt[2][i][0];
			pt[2][i][1].v.y = _size;

			pt[3][i][0].v.x = 0;
			pt[3][i][0].v.y = 0;
			pt[3][i][0].v.z = i*delta;
			pt[3][i][0].c = yc;
			pt[3][i][1] = pt[3][i][0];
			pt[3][i][1].v.y = _size;


			pt[4][i][0].v.x = 0;
			pt[4][i][0].v.y = i*delta;
			pt[4][i][0].v.z = 0;
			pt[4][i][0].c = zc;
			pt[4][i][1] = pt[4][i][0];
			pt[4][i][1].v.z = _size;

			pt[5][i][0].v.x = i*delta;
			pt[5][i][0].v.y = 0;
			pt[5][i][0].v.z = 0;
			pt[5][i][0].c = zc;
			pt[5][i][1] = pt[5][i][0];
			pt[5][i][1].v.z = _size;
		}

		SetTexMode(TM_DISABLE);
		SetVertexSource(pt, sizeof(VC), VC::FORMAT, 120);
		for(int i=0; i<120; i+=2)
		{
			DrawPrimitive(E_LINESTRIP, i, 2);
		}
		SetTexMode(TM_MODULATE);
	}


	void Graphics::InitTexPool(const Path & _folder, void * _context, LOAD_TEX_FUNC _func)
	{
		if(tex_pool)
		{
			delete tex_pool;
		}
		tex_pool = enew e::TexPool(this, _folder, _context, _func);
	}

	TexPool::TexPool(Graphics * _g, const Path & _folder, void * _context, LOAD_TEX_FUNC _func)
	{
		g = _g;
		load_tex_func = _func; 
		load_tex_func_context = _context;

		indexer = enew PathIndexer(_folder, L"*.png");
	}
	
	TexPool::~TexPool()
	{
		delete indexer;
		clear(true);
	}

	Tex * TexPool::Load(const string & _name, bool _delay)
	{ 
		Tex * tex;
		Map<stringa, Tex*>::iterator it = pool.find(_name);
		if(it == pool.end())
		{
			Path path = indexer->GetPath(_name);
			bool is_file = FS::IsFile(path);
//#ifdef NB_DEBUG
//			if(!is_file && !_name.empty() && _name[0]!= L'!')
//			{
//				path = indexer->GetPath(L"!" + _name);
//				is_file = FS::IsFile(path);
//			}
//#endif
			if(is_file)
			{
				tex = g->LoadTexFromFile(path, _delay);
			}
			else
			{
				TexLoader * loader = 0;
				if(load_tex_func)
				{
					loader = load_tex_func(load_tex_func_context, _name);
				}

				if(loader)
				{
					tex = g->LoadTexFromLoader(loader, _delay);
				}
				else
				{
					tex = g->LoadTexFromLoader(enew ErrorTexLoader(_name), _delay);
				}
			}
			
			pool[_name] = tex;
			if(tex)
			{
				tex->AddRef();
				if(!_delay)
				{
					tex->Load();
				}
			}
		}
		else
		{
			tex = it->second;
			if(tex != 0 && !_delay)
			{
				tex->Load();
			}
		}

		return tex;
	}

	void TexPool::clear(bool _force_clear_all)
	{
		Map<stringa, Tex*>::iterator it = pool.begin();
		while(it != pool.end())
		{
			if(it->second && (_force_clear_all || it->second->RefCount() == 1))
			{
				it->second->Release();
				it = pool.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

}
