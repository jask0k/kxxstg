
#pragma once

#include <nbug/core/def.h>
#include <nbug/gl/fbo.h>
#include <nbug/gl/tex.h>
#include <nbug/gl/color.h>
#include <nbug/gl/font/font.h>
#include <nbug/math/math.h>

#undef ZERO
#undef ONE

namespace e
{
	typedef uint32 InternalColor;
	class Graphics;
	class Image;

	enum BlendMode
	{
		BM_DISABLE,  // same as Disable(GS_BLEND)
		BM_NORMAL, 
		BM_ADD,  // for glow effect
		BM_INVERT,
	};

	enum TexMode
	{
		TM_DISABLE,
		// Arg0 = ??
		// Arg1 = ??
		TM_REPLACE,   // Arg0
		TM_MODULATE,  // Arg0 * Arg1 (default)
		//TM_ADD,       // Arg0 + Arg1
		//TM_ADDSIGNED, // Arg0 + Arg1 - 0.5
		//TM_SUBTRACT,  // Arg0 - Arg1
	};

	//enum MatrixStackID
	//{
	//	MT_MODELVIEW = 0,
	//	MT_PROJECTION = 1,
	//};

	enum VertexType
	{
		VT_N = 0x00000001,
		VT_C = 0x00000002,
		VT_T = 0x00000004,
		_VT_MASK = 0x00000007,
	};

	struct V
	{
		Vector3 v;
		static const int FORMAT = 0;
	};

	struct VC
	{
		Vector3 v;
		InternalColor c;
		static const int FORMAT = VT_C;
	};

	struct VT
	{
		Vector3 v;
		Vector2 t;
		static const int FORMAT = VT_T;
	};

	struct VCT
	{
		Vector3 v;
		InternalColor c;
		Vector2 t;
		static const int FORMAT = VT_C | VT_T;
	};

	struct VN
	{
		Vector3 v;
		Vector3 n;
		static const int FORMAT = VT_N;
	};

	struct VNC
	{
		Vector3 v;
		Vector3 n;
		InternalColor c;
		static const int FORMAT = VT_C | VT_N;
	};

	struct VNT
	{
		Vector3 v;
		Vector3 n;
		Vector2 t;
		static const int FORMAT = VT_T | VT_N;
	};

	struct VNCT
	{
		Vector3 v;
		Vector3 n;
		InternalColor c;
		Vector2 t;
		static const int FORMAT = VT_C | VT_T | VT_N;
	};

	enum Primitive
	{
		E_POINTLIST,
		E_LINELIST,
		E_LINESTRIP,
		E_TRIANGLELIST,
		E_TRIANGLESTRIP,
		E_TRIANGLEFAN,
		_E_PRIMITIVE_MAX,
	};

	//typedef bool (*E_LOAD_TEX_FUNC)(void * _param, const stringa & _name, Image &_pic);

	enum GraphicsState
	{
		GS_BLEND,
		GS_SCISSOR_TEST,
		GS_FOG,
		GS_LIGHTING,
		GS_DEPTH_TEST,
		GS_CULL_FACE,
		GS_WIRE_FRAME,
	};

	struct Light
	{
		uint32 type; // 1 = point, 2 = spot, 3 = directional
		RGBA diffuse;
		RGBA specular;
		RGBA ambient;
		Vector3 position;
		Vector3 direction;
		struct
		{
			float constant;
			float linear;
			float quadratic;
		}attenuation;
	};

	struct RAW_MATERIAL
	{
		RGBA diffuse;
		RGBA ambient;
		RGBA specular;
		RGBA emissive;
		float specular_power;
		float alpha;
		int render_mode;
	};

	struct Material : public RAW_MATERIAL
	{
		string tex_name;
		TexRef tex;
		void LoadDefault()
		{
			diffuse.r = diffuse.g = diffuse.b = 0.5f;
			ambient.r = ambient.g = ambient.b = 0.2f;
			specular.r = specular.g = specular.b = 0.7f;
			emissive.r = emissive.g = emissive.b  = 0.1f;
			diffuse.a = ambient.a = specular.a = diffuse.a = 1.0f;
			specular_power = 20;
			render_mode = 2;
			alpha = 1.0f;
			tex_name.c_str();
			tex = 0;
		}
	};

}
