
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Gallery : public StageBG
	{
	public:
		RGBA fog_color;
		Vector2 prevPlayerCenter;
		float texw;
		float texh;
		float texdelta;
		TexRef bgTex0;
		Matrix4 projectionMatrix3D;
		Matrix4 modelViewMatrix3D;
		Gallery();
		~Gallery();
		bool Step() override;
		void Render() override;
		void Calc3DMatries();
	};
}
