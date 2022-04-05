
#pragma once

namespace e
{
	class Pass2
	{
		friend class KxxWin;
		float w;
		float h;
		Graphics * g;
		FboRef fbo;
		ShaderRef shader;
		uintx explode_n_loc;
		uintx explode_loc;
		uintx y_scale_loc;
		Vector4 explode[10];
		uint32 explode_timer[10];
		int explode_n;
	public:
		Pass2();
		~Pass2();
		bool Init(Graphics * _g, float _w, float _h);
		void AddExplode(float _x, float _y);
		void Step();
		void Render();
	};	
}

