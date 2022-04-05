
// #include "../config.h"
#include <z_kxx/enemy/stage_bonus.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	static const int STAGE_BONUS_APPEAR_TIME = 20;
	StageBonus::StageBonus(TexRef _tex, uint32 _delay, uint32 _total_dealy)
	{
		E_ASSERT(_delay <0x7FFFFFFF);
		delay = _delay;
		timer = delay;
		keep_time = _total_dealy - STAGE_BONUS_APPEAR_TIME - _delay;
		E_ASSERT(keep_time <0x7FFFFFFF);
		tex = _tex;
		this->hsz.x = 10;
		this->hsz.y = this->hsz.x * tex->H() / tex->W();
		this->scl.x = 0;
		this->clr.a = 0;

		this->ethereal = true;
		init_life = life = 1;
		state = 0;
	}


	StageBonus::~StageBonus()
	{
	}

	bool StageBonus::Step()
	{
		switch(state)
		{
		case 0:
			if(--timer==0)
			{
				state = 1;
				timer = STAGE_BONUS_APPEAR_TIME;
				kxxwin->PlaySE("stage-bonus", this->pos);
			}
			break;
		case 1:
			if(--timer==0)
			{
				state = 2;
				timer = keep_time;
			}
			else
			{
				scl.x+= 1.0f / STAGE_BONUS_APPEAR_TIME;
				clr.a+= 1.0f / STAGE_BONUS_APPEAR_TIME;
			}
			break;
		case 2:
			if(--timer == 0)
			{
				timer = delay;
				state = 3;
			}
			break;
		case 3:
			if(--timer == 0)
			{
				life = 0;
				state = 4;
			}
			break;
		}
		return true;
	}

}

