
// #include "../config.h"
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/shot/uniform_shot.h>
#include <z_kxx/shot/laser_fan.h>
#include <z_kxx/shot/lapillus_shot.h>
#include <z_kxx/shot/volcano_shot.h>
#include <z_kxx/shot/accelerate_shot.h>
#include <z_kxx/shot/worm_shot.h>
#include <z_kxx/shot/comet_shot.h>
//#include <z_kxx/shot/throw_shot.h>


namespace e
{

#ifdef E_CFG_LUA
	e::Map<int, int> g_enemyShotMap; // shotType : scriptFuncID
	int LuaRegisterEnemyShot(lua_State * L)
	{
		if(lua_gettop(L) != 2)
		{
			E_LUA_ERROR(L, "arg count must be 2.");
		}
		if(!lua_isnumber(L, 1))
		{
			E_LUA_ERROR(L, "type of arg1 must be integer.");
		}
		int shotType = lua_tointeger(L, 1);
		E_ASSERT(lua_isfunction(L, 2));
		int funcID = luaL_ref(L, LUA_REGISTRYINDEX); // -1
		g_enemyShotMap[shotType] = funcID;
		return 0;
	}
#endif

	void AddEnemyShot(Vector2 & _pt, int _type)
	{

#ifdef E_CFG_LUA
		{
			lua_State * L = kxxwin->L;
			e::Map<int, int>::iterator it = g_enemyShotMap.find(_type);
			if(it != g_enemyShotMap.end())
			{
				lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
				E_ASSERT(lua_isfunction(L, -1));
				lua_pushnumber(L, _pt.x);
				lua_pushnumber(L, _pt.y);
				kxxwin->L.Call(2, 0);
				return;
			}
		}
#endif

		int d = kxxwin->state.level;
		int d1 = d+1;
		switch(_type)
		{
		default:
		case ES_SNIPE_0:
			{
				float a = (kxxwin->player->pos - _pt).Angle();
				Vector2 v0, v1;
				v1.SetPolar(shot_vel(40), a);
				v0 = v1 * 3.0f;
				TexRef tex = kxxwin->shotTex[11][0];
				AccelerateShot * p = enew AccelerateShot(tex, v0, v1, 0.3f
					, kxxwin->shotSmokeTex0, 20, 10, 0.3f);
				p->pos = _pt;
				p->Step();
				kxxwin->AddEnemyShotToList(p);
				kxxwin->PlaySE("enemy-shot-0", _pt, 0.5f);
			}
			break;
		case ES_KEDAMA_SCATTER:
			kxxwin->AddPieEnemyShot(_pt, shot_vel(40), d1 * 6, d1 * 6, 10, false);
			kxxwin->PlaySE("enemy-shot-0", _pt, 0.5f);
			break;
		case ES_KEDAMA_SNIPE:
			kxxwin->AddPieEnemySnipe(_pt, shot_vel(50), 20, d+1, 11, false);
			kxxwin->PlaySE("enemy-shot-0", _pt, 0.5f);
			break;
		case ES_LING_SCATTER:
			kxxwin->AddPieEnemyShot(_pt, kxxwin->GetLogicTimer() * 0.025f, shot_vel(40), d*6 + 8, d*6 + 8, 0, true);
			kxxwin->PlaySE("enemy-shot-0", _pt, 0.3f);
			break;
		case ES_LING_SNIPE:
			kxxwin->AddPieEnemySnipe(_pt, shot_vel(30), d*6 + 8, d*6 + 8, 1, true);
			kxxwin->PlaySE("enemy-shot-0", _pt, 0.3f);
			break;
		case ES_OPPOSITE_SPIRE:
			break;
		case ES_LINE_MESS:
			{
				int m = 20 - d;
				int n = 80 - d * 10;
				TexRef tex = kxxwin->shotTex[5][0];
				int x = m + logic_random_int() % n;
				Vector2 speed0, speed1;
				while(x < 460)
				{
					speed1.y = shot_vel(70);
					speed0.y = speed1.y * 2.0f;
					AccelerateShot * b = enew AccelerateShot(tex, speed0, speed1, 0.4f);
					b->SetSize(20);
					b->isEllipseCollision = true;
					b->collisionFrac.x = 0.35f;
					b->collisionFrac.y = 0.35f;
					b->pos.y = _pt.y;
					b->pos.x = float(x);
					//b->RotateNorthToVectorDirection(b->speed);
					x+= m + logic_random_int() % n;
					kxxwin->AddFlashSpark(b->pos.x, b->pos.y, b->hue);
					kxxwin->AddEnemyShotToList(b);
				}
				kxxwin->PlaySE("enemy-shot-2", _pt);
			}
			break;
		case ES_SNIPE_ARRAY:
			{
				TexRef tex = kxxwin->shotTex[8][0];
				float angles[128];
				float snipeAngle = (kxxwin->player->pos - _pt).Angle();
				CalcCircleShot(angles, 40, snipeAngle);
				Vector2 speed0, speed1;
				for(int i = 35; i < 40; i++)
				{
					speed1.SetPolar(shot_vel(70), angles[i]);
					speed0 = speed1*2.0f;
					AccelerateShot * b = enew AccelerateShot(tex, speed0, speed1, 0.4f
						, kxxwin->shotSmokeTex0, 20, 10, 0.3f);
					b->isEllipseCollision = true;
					b->collisionFrac.x = 0.35f;
					b->collisionFrac.y = 0.35f;
					b->pos = _pt;
					b->Step();
					kxxwin->AddEnemyShotToList(b);
				}
				kxxwin->PlaySE("enemy-shot-1", _pt, 0.5f);
			}
			break;
		case ES_4ROSE:
			//{
			//	Animation * ifg = &kxxwin->ifgShot[3][3];
			//	for(int i=0; i<8; i++)
			//	{
			//		float a0 = i * PI * 0.25f;
			//		b = enew EnemyShot();
			//		b->SetSize(20);
			//		b->allowOutOfScreen = true;
			//		b->isEllipseCollision = true;
			//		b->collisionFrac.x = 0.75f;
			//		b->collisionFrac.y = 0.75f;
			//		b->pos = _pt;
			//		track = b->AddSection(ifg);
			//		track->SetRose(4, 300, 0, a0, PI * 0.1f, 0, -PI * 0.1f);
			//		track->timeSpan = S2F(10.0f);
			//		kxxwin->AddEnemyShotToList(b);
			//	}
			//	kxxwin->AddFlashSpark(_enemy);
			//	kxxwin->PlaySE("enemy-shot-1", 0.5f);
			//}
			break;
		case ES_LAPILLUS:
			for(int i=0; i<d1+5; i++)
			{
				float a0 = -PI * logic_random_float();
				LapillusShot * b = enew LapillusShot();
				b->SetSize(20);
//				b->allowOutOfScreen = false;
				b->isEllipseCollision = true;
				b->collisionFrac.x = 0.35f;
				b->collisionFrac.y = 0.35f;
				b->pos = _pt;
				b->speed.SetPolar(shot_vel(50), a0);
				b->gravity.y = sin(a0 + PI) * shot_vel(50) * 0.005f;
				kxxwin->AddEnemyShotToList(b);
			}
			break;
		case ES_LASER_FAN:
			{
				LaserFan * p = enew LaserFan(_pt, 8*d1);
				kxxwin->AddEnemyShotToList(p);
			}
			break;
		case ES_VOLCANO:
			{
				E_ASSERT(0);
				//VolcanoShot * p = enew VolcanoShot(50 - d*10);
				//p->pos = _pt;
				//p->speed = (kxxwin->player->pos - _pt).GetNormal() * PS2PF(kxx_s2[1]);
				//kxxwin->AddEnemyShotToList(p);
				//kxxwin->PlaySE("enemy-shot-2", _pt, 0.5f);
			}
			break;
		case ES_DS2_2:
			{
				TexRef tex1 = kxxwin->shotTex[1][0]; // card
				TexRef tex2 = kxxwin->shotTex[0][0]; // petal
				Vector2 speed0, speed1;
				speed0.x = 0;
				speed1.x = 0;
				for(int i=0; i<d1; i++)
				{
					int direction = logic_random_int() & 0x01;
					//float sf = logic_random_float() + 1.0f;
					float sf = shot_vel(50);
					if(direction)
					{
						sf = -sf;
					}
					speed0.y = sf;
					speed1.y = -speed0.y;
					// card shot
					AccelerateShot * b = enew AccelerateShot(tex1, speed0, speed1, 8.0f / d1);
					b->isEllipseCollision = true;
					b->collisionFrac.x = 0.35f;
					b->collisionFrac.y = 0.35f;
					b->pos.x = _pt.x + (logic_random_float() - 0.5f) * 180;
					b->pos.y = _pt.y + (logic_random_float() - 0.5f) * 80;
					b->Step();
					kxxwin->AddEnemyShotToList(b);

					// water drop
					speed1.y = -speed1.y;
					speed0.y = speed1.y * 1.2f;
					b = enew AccelerateShot(tex2, speed0, speed1, 3.0f / d1
						, kxxwin->shotSmokeTex0, 20, 10, 0.3f);
					b->isEllipseCollision = true;
					b->collisionFrac.x = 0.35f;
					b->collisionFrac.y = 0.35f;
					b->pos.x = _pt.x + (logic_random_float() - 0.5f) * 180;
					b->pos.y = _pt.y + (logic_random_float() - 0.5f) * 80;
					b->Step();
					kxxwin->AddEnemyShotToList(b);
				}
				kxxwin->PlaySE("enemy-shot-0", _pt, 0.5f);
			}
			break;
		case ES_WORM:
			{
				
				float angles[64];
				static bool b[5][11] = 
				{
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
					{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
					{0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0},
				};
				CalcCircleShot(angles, 12, PI * 0.5f);
				for(int i = 0; i < 11; i++)
				{
					if(b[d1][i])
					{
						GuideWormShot * b = enew GuideWormShot(kxxwin->shotTex[7][0], 10, 120, _pt, shot_vel(50), angles[i], 3.6f, 3.6f);
						kxxwin->AddEnemyShotToList(b);
					}
				}
				kxxwin->PlaySE("enemy-shot-laser", _pt, 0.5f);
			}
			break;
		case ES_SHOTGUN:
			{
				TexRef tex;
				int c = 10 + 8 * d;
				float a0 = (kxxwin->player->pos - _pt).Angle();
				Vector2 speed0, speed1;
				for(int i=0; i<c; i++)
				{
					float power = logic_random_float();
					float a = logic_random_float() * power;
					if(i & 0x01)
					{
						a = a0 +  a;
					}
					else
					{
						a = a0 -  a;
					}
					float s = shot_vel(15);
					s = s * (1.0f - power*0.5f);
					float r = 10 * (0.4f + power*0.6f);
					speed1.SetPolar(s, a);
					speed0 = speed1*3.0f;
					tex = power >= 0.5f ? kxxwin->shotTex[10][0] : kxxwin->shotTex[11][0];
					AccelerateShot * b = enew AccelerateShot(tex, speed0, speed1, 0.3f);
					b->isEllipseCollision = true;
					b->collisionFrac.x = 0.35f;
					b->collisionFrac.y = 0.35f;
					b->pos = _pt;
					b->hsz.y = b->hsz.x = r;
					b->Step();
					kxxwin->AddEnemyShotToList(b);
				}
			}
			break;
		case ES_COMET:
			{
				CometShot * p = enew CometShot();
				p->pos = _pt;
				p->speed = shot_vel(50);
				p->SetSize(41);
				p->ang = (kxxwin->player->pos - _pt).Angle();
				kxxwin->PlaySE("enemy-shot-2", _pt, 0.5f);
				kxxwin->AddEnemyShotToList(p);
			}
			break;
		}
	}
}
