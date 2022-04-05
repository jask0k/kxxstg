
#include <z_kxx/stage/simple_stage_bg.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player.h>
//#include <z_kxx/enemy/enemy.h>
//#include <z_kxx/stage/kxx_dialog.h>
//#include <z_kxx/boss/a/boss_a.h>
//#include <z_kxx/boss/a/boss_a_mid.h>
#include <z_kxx/util/util_func.h>

namespace e
{
	SimpleStageBG::SimpleStageBG(int _stage_id)
	{
		opacity = true;
		fog_color.r = 0.3f;
		fog_color.g = 0.2f;
		fog_color.b = 0.0f;
		fog_color.a = 0.2f;

		prevPlayerCenter.x = K_GAME_W * 0.5f;
		prevPlayerCenter.y = 0;
		bgTex0 = kxxwin->LoadTex(L"stage-" + string(_stage_id) + "-terrain");
		texw=(float)bgTex0->W();
		texh=(float)bgTex0->H();
		texdelta = 0;
		Calc3DMatries();
	}

	SimpleStageBG::~SimpleStageBG()
	{
	}

	bool SimpleStageBG::Step() 
	{
		texdelta+=0.5f;
		if(texdelta >= texh)
		{
			texdelta = 0;
		}
		return true;
	}

	void SimpleStageBG::Calc3DMatries()
	{
		graphics->CalcPerspective(projectionMatrix3D, PI * 0.3f, 1.2f, 1, 1000);
		float y = prevPlayerCenter.y > K_GAME_H ? K_GAME_H : prevPlayerCenter.y;
		Vector3 eye = {(K_GAME_W*0.5f - prevPlayerCenter.x)*0.2f, y*0.25f, -250};
		Vector3 lookAt = {0, -100 + y*0.1f, 0};
		Vector3 up = {0, 0, -1};
		graphics->CalcLookAt(modelViewMatrix3D, eye, lookAt, up);
	}

	void SimpleStageBG::Render()
	{
		if(!kxxwin->isPause && kxxwin->GetRenderTimer() % 2 == 0 && prevPlayerCenter != kxxwin->player->pos)
		{
			PositionApproach(prevPlayerCenter, kxxwin->player->pos, 5);
			Calc3DMatries();
		}

		graphics->SetProjectViewMatrix(projectionMatrix3D, modelViewMatrix3D);
		graphics->SetFogParam(fog_color.r, fog_color.g, fog_color.b, fog_color.a, 180, 400, 0);
		graphics->Enable(GS_FOG);

		graphics->SetColor(0xffffffff);
		graphics->BindTex(bgTex0);
		float ymin = -600 + texdelta;
		float ymax = 150;
		float xmin = -390;
		float xmax = 300;
		for(float y = ymin; y<= ymax; y+= texh)
		{
			for(float x = xmin; x<= xmax; x+= texw)
			{
				float x1 = x+texw;
				float y1 = y+texh;
				graphics->DrawQuad(
					x, y, 0, 0, 0,
					x, y1, 0, 0, 1,
					x1, y1, 0, 1, 1,
					x1, y, 0, 1, 0);
			}
		}
		graphics->Disable(GS_FOG);
	}
}

