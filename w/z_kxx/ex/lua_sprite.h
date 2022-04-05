#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
#ifdef E_CFG_LUA
	class LuaSprite : public Shot
	{
		lua_State * L;
		int idTable;
		//int idCollide;
		//int idOnHit;
		//int idDropItem;
		int idStep;
		int idRender;
		LuaSprite()
		{}
		~LuaSprite();
		static LuaSprite * _New(lua_State * L);
	public:
		static int NewSpark(lua_State * L);
		//static int NewPlayerShot(lua_State * L);
		//bool Collide(const Vector2 & _v, float _r) override;
		//bool OnHit(const Vector2 & _pt) override;
		//void DropItem() override;
		bool Step() override;
		void Render() override;
	};
#endif
}

