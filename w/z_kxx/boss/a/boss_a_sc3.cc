#include <z_kxx/boss/a/boss_a_sc3.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/shot/uniform_shot.h>

namespace e
{
	class BossASC3MasterShot : public EnemyShot
	{
		bool end;
	public:
		static const int MAX_EDGE_COUNT = 20;
		Vector2 v[MAX_EDGE_COUNT][4];
		
		int edge_count;
		float da;
		TexRef sub_shot_tex;
		uint32 shot_timer;
		float radius;
		BossASC3MasterShot(float _radius, int _edge_count, float _da)
		{
			fragile = false;
			da = _da;
			end = false;
			radius = _radius;

			shot_timer = 1;

			edge_count = _edge_count;
			float w = 10.0f;
			float delta = 2.0f * PI / edge_count;
			float a = 0;
			for(int i=0; i<edge_count; i++, a+=delta)
			{
				float cos_a = cos(a);
				float sin_a = sin(a);
				v[i][0].x = (_radius - w) * cos_a;
				v[i][0].y = (_radius - w) * sin_a;
				v[i][1].x = (_radius + w) * cos_a;
				v[i][1].y = (_radius + w) * sin_a;
				if(i > 0)
				{
					v[i][3] = v[i-1][0];
					v[i][2] = v[i-1][1];
				}
			}
			v[0][3] = v[edge_count-1][0];
			v[0][2] = v[edge_count-1][1];
		}
		
		~BossASC3MasterShot()
		{
		}
		
		void End()
		{
			end = true;
		}

		void DropItem() override
		{
			for(int i = 0; i < edge_count; i++)
			{
				Vector2 * pv = v[i];
				kxxwin->AddDrop(DROP_TINY_POINT, pv[0].x, pv[0].y, false, -(PI * 0.5f));
			}
		}

		void Render() override
		{
			graphics->SetColor(clr);
			graphics->SetBlendMode(blm);
		//	graphics->SetTexMode(TM_DISABLE);
			graphics->PushMatrix();
			graphics->TranslateMatrix(pos.x, pos.y, 0);
			graphics->RotateMatrix(ang, 0, 0, 1);
			graphics->BindTex(tex);

			//graphics->DrawQuad(
			//		0, 0, 0, 0, 0,
			//		100,0, 0, 1, 0, 
			//		100, 100, 0, 1, 1, 
			//		0, 100, 0, 0, 1
			//		);

			for(int i = 0; i < edge_count; i++)
			{
				Vector2 * pv = v[i];
				graphics->DrawQuad(
					pv[0].x, pv[0].y, 0, 0, 0,
					pv[1].x, pv[1].y, 0, 1, 0, 
					pv[2].x, pv[2].y, 0, 1, 1, 
					pv[3].x, pv[3].y, 0, 0, 1
					);
			}

		//	graphics->SetTexMode(TM_MODULATE);
			
			graphics->PopMatrix();
		}

		bool Step() override
		{
			ang+= da;
			if(--shot_timer == 0)
			{
				shot_timer = (16 - 3 * kxxwin->state.level) * K_LOGIC_FPS_MUL;
				float a = ang;
				float delta = 2.0f * PI / edge_count;
				for(int i=0; i<edge_count; i++, a+=delta)
				{
					float cos_a = cos(a);
					float sin_a = sin(a);

					UniformShot * p = enew UniformShot(sub_shot_tex);
					p->ang = a - 0.5f * PI;
					p->pos.x = pos.x + radius * cos_a;
					p->pos.y = pos.y + radius * sin_a;
					p->collisionFrac.x = 0.35f;
					p->collisionFrac.y = 0.35f;
					p->SetSize(20);
					float speed_f = shot_vel(20);
					p->speed.x = speed_f * cos_a;
					p->speed.y = speed_f * sin_a;
					kxxwin->AddFlashSpark(p->pos.x, p->pos.y, p->hue);
					kxxwin->AddEnemyShotToList(p);
				}
				kxxwin->PlaySE("enemy-shot-0", this->pos, 0.2f);
			}
			return !end;
		}
	};

	BossASC3::BossASC3()
	{
		//sc_name = _TT("Dark Matter");
	}

	void BossASC3::Begin()
	{
		this->time_limit = 30 * K_LOGIC_FPS;

		shot_timer = S2F(1.25f);

		kxxwin->PlaySE("spell-card", boss->pos);
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				p->state = ES_LING_SNIPE;
				p->ethereal = false;
			}
		}
		boss->ShowSCName(sc_name);
		boss->ShowAvatar();
		boss->ShowRing(true);
		kxxwin->AddBlastWave(boss->pos, false);

		{
			BossASC3MasterShot * p = enew BossASC3MasterShot(100, 3, 0.0025f);
			p->pos = boss->pos;
			p->tex = kxxwin->shotTex[10][0];
			p->SetSize(10);
			p->clr.b = 0.0f;
			p->sub_shot_tex = kxxwin->shotTex[0][0];
			kxxwin->AddEnemyShotToList(p, false, RL_GAME_BOSS);
			ms1 = p;
		}

		{
			BossASC3MasterShot * p = enew BossASC3MasterShot(100, 3, 0.0025f);
			p->pos = boss->pos;
			p->tex = kxxwin->shotTex[10][0];
			p->ang = PI / 3.0f;
			p->SetSize(10);
			p->clr.b = 0.0f;
			p->sub_shot_tex = kxxwin->shotTex[0][0];
			kxxwin->AddEnemyShotToList(p, false, RL_GAME_BOSS);
			ms2 = p;
		}

		{
			BossASC3MasterShot * p = enew BossASC3MasterShot(100, 3, -0.0025f);
			p->pos = boss->pos;
			p->tex = kxxwin->shotTex[10][0];
			p->SetSize(10);
			p->clr.r = 0.0f;
			p->sub_shot_tex = kxxwin->shotTex[11][0];
			kxxwin->AddEnemyShotToList(p, false, RL_GAME_BOSS);
			ms3 = p;
		}
		{
			BossASC3MasterShot * p = enew BossASC3MasterShot(100, 3, -0.0025f);
			p->pos = boss->pos;
			p->tex = kxxwin->shotTex[10][0];
			p->ang = -PI / 3.0f;
			p->SetSize(10);
			p->clr.r = 0.0f;
			p->sub_shot_tex = kxxwin->shotTex[11][0];
			kxxwin->AddEnemyShotToList(p, false, RL_GAME_BOSS);
			ms4 = p;
		}

	}

	bool BossASC3::Step()
	{
		if(--shot_timer==0)
		{
			shot_timer = S2F(1.25f);
		}
		return true;
	}

	void BossASC3::End()
	{
		SCScript::End();
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				p->state = -1;
				p->ethereal = true;
			}
		}
		boss->ShowRing(false);
		kxxwin->ClearEnemyShots(false, 10);
		boss->SetDark(100.0f, 0.1f);
		ms1->End();
		ms2->End();
		ms3->End();
		ms4->End();
		boss->HideSCName();
	}
}
