
// #include "../config.h"
#include <z_kxx/sprite/general_aura.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	// E_DEFINE_MEMORY_POOL(GeneralAura);

	GeneralAura::GeneralAura(TexRef _tex)
	{
		tex = _tex;
		//if(tex)
		//{
		//	tex->AddRef();
		//}
	}


	GeneralAura::~GeneralAura()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void GeneralAura::Render()
	{
		// graphics->BlendOn();
		graphics->SetBlendMode(BM_ADD);
		//graphics->SetBlendMode(BM_NORMAL);
		graphics->SetColor(clr);
		uint32 localTimeTick = kxxwin->GetRenderTimer();
		float a = localTimeTick * -0.10f / 1;
		float scale = abs(int(localTimeTick & 0xff) - 128)/ 256.0f + 1.0f;
		float w = hsz.y * scale;
		graphics->PushMatrix();
		graphics->TranslateMatrix(this->pos.x, this->pos.y, 0);
		graphics->RotateMatrix(a, 0, 0, 1);
		graphics->BindTex(tex);
		graphics->DrawQuad(-w, -w, w, w);
		graphics->PopMatrix();
		graphics->SetBlendMode(BM_NORMAL);
	}

	bool GeneralAura::Step()
	{
		return true;
	}
}
