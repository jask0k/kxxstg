#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	// owner is C++, not lua
#ifdef E_CFG_LUA
	class LuaShot : public Shot
	{
		lua_State * L;
		int idTable;
		int idCollide;
		int idOnHit;
		int idDropItem;
		int idStep;
		int idRender;
		LuaShot()
		{
			pos.x = 30;
			pos.y = 40;
		}
		~LuaShot();
		static LuaShot * NewLuaShotFromTable(lua_State * L);
		//static int LuaShot_gc(lua_State * L);
		static int NewEnemyShot(lua_State * L);
		static int NewPlayerShot(lua_State * L);
	public:
		static bool Register(lua_State * L);
		//static int AllocLuaShot(lua_State * L);
		bool Collide(const Vector2 & _v, float _r) override;
		bool OnHit(const Vector2 & _pt) override;
		void DropItem() override;
		bool Step() override;
		void Render() override;
	};
#endif
}

