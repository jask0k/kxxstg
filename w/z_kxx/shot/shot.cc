
// #include "../config.h"
#include <z_kxx/shot/shot.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/sprite/shot_crash.h>
#include <z_kxx/sprite/spark1.h>

namespace e
{
	Shot::Shot()
	{
		dead = false;
		power = 0;
		collisionFrac.x = 0.5f;
		collisionFrac.y = 0.5f;
		isEllipseCollision = true;
		unavailableTimer = 0;
		fragile = true;
	}

	static inline Vector2 CalcRotatedEllipseFocus_X(float a, float b, float ang)
	{
		float f = sqrt(a*a - b*b);
		Vector2 vf = {f * cos(ang), f * sin(ang)};
		return vf;
	}

	static inline Vector2 CalcRotatedEllipseFocus_Y(float a, float b, float ang)
	{
		float f = sqrt(b*b - a*a);
		Vector2 vf = {f * sin(-ang), f * cos(-ang)};
		return vf;
	}

	bool Shot::Collide(const Vector2 & _v, float _r)
	{
		float a = hsz.x * scl.x * collisionFrac.x + _r; 
		float b = hsz.y * scl.y * collisionFrac.y + _r;
		if(isEllipseCollision)
		{
			if(a == b)
			{
				return (_v - pos).LengthSquared() <= a*a;
			}
			else if(a < b)
			{
				Vector2 vf = CalcRotatedEllipseFocus_Y(a, b, ang);
				float d = (_v - (pos + vf)).length() + (_v - (pos - vf)).length();
				return d <= 2 * b;
			}
			else
			{
				Vector2 vf = CalcRotatedEllipseFocus_X(a, b, ang);
				float d = (_v - (pos + vf)).length() + (_v - (pos - vf)).length();
				return d <= 2 * a;
			}
		}
		else
		{
			// rotate test point to shots coord
			Vector2 v1 = _v - pos;
			float angle_v1 = v1.Angle();
			float da = angle_v1-ang;
			float sin_da = cos(da);
			float cos_da = sin(da);
			float r = v1.length();
			v1.x = r * sin_da;
			v1.y = r * cos_da;

			// rect test
			return  v1.x >= -a && v1.x <= a && v1.y >= -b && v1.y <= b;
		}
	}

	void Shot::OnHit(const Vector2 & _pt)
	{
		kxxwin->AddSmallSpark(pos, 2, hue);
	}

	void Shot::OnCrash(const Vector2 & _pt)
	{
		if(tex)
		{
			ShotCrash * p = enew ShotCrash(tex);
			p->pos = pos;
			p->ang = ang;
			p->blm = BM_ADD;
			p->hue = hue;
			kxxwin->AddSparkToList(p);
		}
	}

	void Shot::DropItem()
	{
		kxxwin->AddDrop(DROP_TINY_POINT, pos.x, pos.y, false, -(PI * 0.5f));
	}

	bool Shot::DisappearTest()
	{
		//return IsOutOfGameArea(pos, 20);
		float xext = scl.x * hsz.x * 1.2f;
		if(xext < 0)
		{
			xext = -xext;
		}
		float yext = scl.y * hsz.y * 1.2f;
		if(yext < 0)
		{
			yext = -yext;
		}
		return pos.x < -xext || pos.x > K_GAME_W + xext || pos.y < -yext || pos.y > K_GAME_H + yext;
	}

	bool Shot::Step()
	{
		if(unavailableTimer)
		{
			unavailableTimer--;
		}
		return !DisappearTest();
	}

	void Shot::OnMasterDestroyed()
	{
		dead = true;
	}

	void Shot::RenderDebug()
	{
#ifdef _NB_DEBUG
		float a = hsz.x * scl.x * collisionFrac.x;
		float b = hsz.y * scl.y * collisionFrac.y;
		RGBA color = kxxwin->debugFontColor;
		if(this->isEllipseCollision)
		{
			graphics->DrawEllipse(pos.x, pos.y, ang, a, b, color, false);
			if(a < b)
			{
				Vector2 vf = CalcRotatedEllipseFocus_Y(a, b, ang);
				graphics->DrawQuad(pos.x+vf.x, pos.y+vf.y, 0, 1, 1, color, true);
				graphics->DrawQuad(pos.x-vf.x, pos.y-vf.y, 0, 1, 1, color, true);
			}
			else if(a > b)
			{
				Vector2 vf = CalcRotatedEllipseFocus_X(a, b, ang);
				graphics->DrawQuad(pos.x+vf.x, pos.y+vf.y, 0, 1, 1, color, true);
				graphics->DrawQuad(pos.x-vf.x, pos.y-vf.y, 0, 1, 1, color, true);
			}
		}
		else
		{
			graphics->DrawQuad(pos.x, pos.y, ang, a, b, color, false);
		}
#endif
	}

	PlayerShot::PlayerShot()
	{
		fire_sputter = kxxwin->player->state.extra_card == 1;
		//puncture = kxxwin->player->state.extra_card == 2;
		puncture = false;
	}

	class FireSputter : public PlayerShot
	{
	public: 
		FireSputter()
		{
			fragile = false;
			fire_sputter = false;
			puncture = true;
			unavailableTimer = 60;
		}

		bool Step() override
		{
			if(unavailableTimer == 0)
			{
				return false;
			}
			unavailableTimer--;
			return true;
		}

		void OnHit(const Vector2 & _pt) override
		{
			Spark1 * p = enew Spark1(kxxwin->fire_sputter_tex, 0.7f, 
				0, 0, 
				12, 0,
				0, 1);
			p->pos.x = _pt.x + 40 * (frand() - 0.5f);
			p->pos.y = _pt.y + 20 * (frand() - 0.5f);
			p->speed.x = 0;
			p->speed.y = PS2PF(-10);
			kxxwin->AddSparkToList(p, RL_GAME_PLAYER_PET);
		}
	};

	void PlayerShot::ActivateSputter()
	{
		if(fire_sputter)
		{
			FireSputter * p = enew FireSputter();
			p->pos = pos;
			p->hsz.x = 80;
			p->hsz.y = 80;
			p->collisionFrac.x = 1.0f;
			p->collisionFrac.y = 1.0f;
			p->power = 0.2f * this->power;
			kxxwin->playerShotList.push_back(p); // invisible
		}
	}

}
