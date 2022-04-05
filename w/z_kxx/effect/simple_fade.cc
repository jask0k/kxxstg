
// #include "../config.h"
#include <z_kxx/effect/simple_fade.h>

namespace e
{
	SimpleFade::SimpleFade(Type _type, bool _ca)
		: Fade(_ca, false)
		, type(_type)
	{
		if(_ca && type == typeShutter)
		{
			overlay = false;
		}
	}


	SimpleFade::~SimpleFade()
	{
	}

	void SimpleFade::Render()
	{
		switch(type)
		{
		case typeAlpha:
		default:
			if(fboA)
			{
				// g->BlendOn();
				g->SetTexMode(TM_MODULATE);
				g->BindTex(fboA->GetTex());
				// g->SetTexMode(TextureMode::Modulate);
				g->SetColor(1, 1, 1, 1.0f-GetFrac());
				g->DrawQuad(0, 0, w, h);
				// g->SetTexMode(TextureMode::replace);
				g->BindTex(0);
			}
			else
			{
				g->SetTexMode(TM_DISABLE);
				g->SetColor(0, 0, 0, GetFrac());
				g->DrawQuad(0, 0, w, h);
				g->SetTexMode(TM_MODULATE);
			}
			break;
		case typeShutter:
			if(fboA)
			{
				uint st = duration / 2;
				uint s0 = 0;
				uint s1 = st;
				uint s3 = duration;

				if(counter < s1)
				{
					// g->BlendOff();
					g->SetTexMode(TM_MODULATE);
					g->BindTex(fboA->GetTex());
					g->DrawQuad(0, 0, w, h);
					g->SetTexMode(TM_DISABLE);
					g->BindTex(0);
					g->SetColor(0, 0, 0, 1);

					static const int SPLIT = 40; // bar count
					static const int STEP  = 20; // transform step of each bar
					static const int TOTAL_STEP = SPLIT + STEP;
					float h1 = h / (float)SPLIT;
					int step = counter * TOTAL_STEP / st;
					for(int i=0; i<SPLIT; i++)
					{
						float h2;
						int j = step - i;
						if(j <= 0)
						{
							continue;
						}
						else if(j >= STEP)
						{
							h2 = h1;
						}
						else
						{
							h2 = h1 * j / STEP;
						}
						float y = h - h1 * i;
						g->DrawQuad(0, y - h2*0.5f, (float)w, y + h2*0.5f);
					}

					g->SetTexMode(TM_MODULATE);
					// g->BlendOn();
				}
				else
				{
					overlay = true;
					// // g->BlendOff();
					//g->SetTexMode(TM_MODULATE);
					//g->BindTex(fboA->GetTex());
					//g->DrawQuad(0, 0, (float)w, (float)h);
					g->SetTexMode(TM_DISABLE);
					g->SetColor(0, 0, 0, 1);

					static const int SPLIT = 40; // bar count
					static const int STEP  = 20; // transform step of each bar
					static const int TOTAL_STEP = SPLIT + STEP;
					float h1 = h / (float)SPLIT;
					int step = (counter-s1) * TOTAL_STEP / st;
					for(int i=0; i<SPLIT; i++)
					{
						float h2;
						int j = step - i;
						if(j <= 0)
						{
							h2 = h1;
						}
						else if(j >= STEP)
						{
							continue;
						}
						else
						{
							h2 = h1 * (STEP-j) / STEP;
						}
						float y = h - h1 * i;
						g->DrawQuad(0, y - h2*1.5f, (float)w, y - h2*0.5f);
					}

					g->SetTexMode(TM_MODULATE);
					// g->BlendOn();
				}
			}
			else
			{
				static const int SPLIT = 40; // bar count
				static const int STEP  = 20; // transform step of each bar
				static const int TOTAL_STEP = SPLIT + STEP;
				float h1 = h / (float)SPLIT;
				int step = (duration-counter) * TOTAL_STEP / duration;

				// g->BlendOff();
				g->SetTexMode(TM_DISABLE);
				g->SetColor(0, 0, 0, 1);

				for(int i=0; i<SPLIT; i++)
				{
					float h2;
					int j = step - i;
					if(j <= 0)
					{
						continue;
					}
					else if(j >= STEP)
					{
						h2 = h1;
					}
					else
					{
						h2 = h1 * j / STEP;
					}
					float y = h1 * i;
					g->DrawQuad(0, y, (float)w, y + h2);
				}

				g->SetTexMode(TM_MODULATE);
				// g->BlendOn();
			}
			break;
		}
	}

}
