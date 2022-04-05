// #include "../config.h"
#include <z_kxx/shot/lua_shot.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
#ifdef E_CFG_LUA
	// expose LuaShot to Lua
	// ================================================
#define CLASSNAME "LuaShot"

	static LuaShot * LuaToLuaShot(lua_State * L, int _index)
	{
		int type = lua_type(L, _index);
		if(type == LUA_TUSERDATA)
		{
			void *ud = lua_touserdata(L, _index);
			if(ud == 0)
			{
				luaL_typerror(L, _index, CLASSNAME);
			}
			return *(LuaShot**)ud;
		}
		else if(type != LUA_TNIL)
		{
			luaL_typerror(L, _index, CLASSNAME);
		}

		return 0;
	}

	// fallback virtual functions
	//bool Collide(const Vector2 & _v, float _r) override;
	static int LuaShot_Collide(lua_State * L)
	{
		int top = lua_gettop(L);
		E_ASSERT(top == 4);
		LuaShot * p = LuaToLuaShot(L, 1);
		Vector2 v;
		v.x = lua_tofloat(L, 2);
		v.y = lua_tofloat(L, 3);
		float r = lua_tofloat(L, 4);
		bool ret = p->Collide(v, r);
		lua_pushboolean(L, ret);
		return 1;
	}

	//bool OnHit(const Vector2 & _pt) override;
	static int LuaShot_OnHit(lua_State * L)
	{
		int top = lua_gettop(L);
		E_ASSERT(top ==3);
		LuaShot * p = LuaToLuaShot(L, 1);
		Vector2 v;
		v.x = lua_tofloat(L, 2);
		v.y = lua_tofloat(L, 3);
		bool ret = p->OnHit(v);
		lua_pushboolean(L, ret);
		return 1;
	}

	//void DropItem() override;
	static int LuaShot_DropItem(lua_State * L)
	{
		int top = lua_gettop(L);
		E_ASSERT(top ==1);
		LuaShot * p = LuaToLuaShot(L, 1);
		p->DropItem();
		return 0;
	}
	//bool Step() override;
	static int LuaShot_Step(lua_State * L)
	{
		int top = lua_gettop(L);
		E_ASSERT(top ==1);
		LuaShot * p = LuaToLuaShot(L, 1);
		bool ret = p->Step();
		lua_pushboolean(L, ret);
		return 1;
	}
	//void Render() override;
	static int LuaShot_Render(lua_State * L)
	{
		int top = lua_gettop(L);
		E_ASSERT(top ==1);
		LuaShot * p = LuaToLuaShot(L, 1);
		p->Render();
		return 0;
	}




	static const luaL_reg LuaShot_methods[] = 
	{
		{"pos", &Sprite::Lua_Pos},
		{"hsz", &Sprite::Lua_Hsz},
		{"scl", &Sprite::Lua_Scl},
		{"ang", &Sprite::Lua_Ang},
		{"blm", &Sprite::Lua_Blm},
		{"clr", &Sprite::Lua_Clr},
		{"tex", &Sprite::Lua_Tex},
		{"Collide",  &LuaShot_Collide},
		{"OnHit",    &LuaShot_OnHit},
		{"DropItem", &LuaShot_DropItem},
		{"Step",     &LuaShot_Step},
		{"Render",   &LuaShot_Render},
		{0, 0}
	};

	//int LuaSetLuaShot(lua_State * L)
	//{
	//	LuaShot * tex = *(LuaShot**) lua_touserdata(L, 1);
	//	Graphics::Singleton()->SetLuaShot(tex);
	//	return 0;
	//}

	/*int LuaLoadLuaShot(lua_State * L)
	{
		E_ASSERT(lua_gettop(L) == 1);
		const char * p = lua_tostring(L, 1);
		LuaShot * tex = Graphics::Singleton()->LoadLuaShot(p);
		if(tex)
		{
			tex->AddRef();
			void * ud = lua_newuserdata(L, sizeof(LuaShot *));
			*(LuaShot**)ud = tex;
			luaL_getmetatable(L, CLASSNAME);
			lua_setmetatable(L, -2);
		}
		else
		{
			lua_pushnil(L);
		}
		return 1;
	}*/

	bool LuaShot::Register(lua_State * L)
	{
		lua_newtable(L);
		int methodtable = lua_gettop(L);
		luaL_newmetatable(L, CLASSNAME);
		int metatable = lua_gettop(L);

		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, methodtable);
		lua_settable(L, metatable); 

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methodtable);
		lua_settable(L, metatable);

		//lua_pushliteral(L, "__gc");
		//lua_pushcfunction(L, LuaShot_gc);
		//lua_settable(L, metatable);

		lua_pop(L, 1);

		luaL_openlib(L, 0, LuaShot_methods, 0);
		lua_pop(L, 1);

		//lua_register(L, "LuaShot", &LuaLoadLuaShot);
		//lua_register(L, "SetLuaShot", &LuaSetLuaShot);

		lua_register(L, "AddEnemyShot", &LuaShot::NewEnemyShot);
		lua_register(L, "AddPlayerShot", &LuaShot::NewPlayerShot);

		return true;
	}

	// bind Lua Table on LuaShot
	// ================================================
	LuaShot * LuaShot::NewLuaShotFromTable(lua_State * L)
	{
		E_ASSERT(lua_istable(L,-1));

		//
		const char * key = "u";
		lua_pushstring(L, key);
		// key/table

		// create LuaShot, register as userdata
		LuaShot * p = enew LuaShot();
		int n = sizeof(LuaShot);
		// p->AddRef(); 
		p->L = L;

		void * ud = lua_newuserdata(L, sizeof(LuaShot *));
		// ud/key/talbe

		*(LuaShot**)ud = p;
		luaL_getmetatable(L, CLASSNAME);
		// mt/ud/key/table

		lua_setmetatable(L, -2);
		// ud/key/table

		// ud bind on table
		lua_settable(L, -3);
		// table

		// Ref of fuctions
		lua_pushstring(L, "Render");
		E_ASSERT(lua_isstring(L,-1));
		lua_gettable(L, -2);
		p->idRender = luaL_ref(L, LUA_REGISTRYINDEX);


		lua_pushstring(L, "Collide");
		E_ASSERT(lua_isstring(L,-1));
		lua_gettable(L, -2);
		p->idCollide = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_pushstring(L, "OnHit");
		E_ASSERT(lua_isstring(L,-1));
		lua_gettable(L, -2);
		p->idOnHit = luaL_ref(L, LUA_REGISTRYINDEX);


		lua_pushstring(L, "DropItem");
		E_ASSERT(lua_isstring(L,-1));
		lua_gettable(L, -2);
		p->idDropItem = luaL_ref(L, LUA_REGISTRYINDEX);


		lua_pushstring(L, "Step");
		E_ASSERT(lua_isstring(L,-1));
		lua_gettable(L, -2);
		p->idStep = luaL_ref(L, LUA_REGISTRYINDEX);
	
		// Ref of table
		p->idTable  =  luaL_ref(L, LUA_REGISTRYINDEX);

		return p;
	}

	int LuaShot::NewEnemyShot(lua_State * L)
	{
		LuaShot * p = NewLuaShotFromTable(L);
		kxxwin->AddEnemyShotToList(p);
		return 0;
	}

	int LuaShot::NewPlayerShot(lua_State * L)
	{
		LuaShot * p = NewLuaShotFromTable(L);
		kxxwin->AddPlayerShotToList(p);
		return 0;
	}

	LuaShot::~LuaShot()
	{
		lua_unref(L, idRender); 
		lua_unref(L, idCollide);
		lua_unref(L, idOnHit);
		lua_unref(L, idDropItem);
		lua_unref(L, idStep);
		lua_unref(L, idTable);
	}


	bool LuaShot::Collide(const Vector2 & _pt, float _r)
	{
		// NB_PROFILE_INCLUDE;
		if(idCollide == LUA_REFNIL)
		{
			return Shot::Collide(_pt, _r);
		}

		// function name
		lua_rawgeti(L, LUA_REGISTRYINDEX, idCollide);
		E_ASSERT(lua_isfunction(L,-1));
		// push self
		lua_rawgeti(L, LUA_REGISTRYINDEX, idTable);
		E_ASSERT(lua_istable(L,-1));
		// push other args
		lua_pushnumber(L, _pt.x);
		lua_pushnumber(L, _pt.y);
		lua_pushnumber(L, _r);
		// call
		int err = lua_pcall(L, 4, 1, 0);
		if(err)
		{
			kxxwin->L.RaiseLuaStackError();
			return false;
		}
		// return
		E_ASSERT(lua_isboolean(L, -1));
		int ret = lua_toboolean(L, -1); 
		lua_pop(L, 1);
		return ret ? true : false;
	}

	bool LuaShot::OnHit(const Vector2 & _pt)
	{
		// NB_PROFILE_INCLUDE;
		if(idOnHit == LUA_REFNIL)
		{
			return Shot::OnHit(_pt);
		}
		// function name
		lua_rawgeti(L, LUA_REGISTRYINDEX, idOnHit);
		E_ASSERT(lua_isfunction(L,-1));
		// push self
		lua_rawgeti(L, LUA_REGISTRYINDEX, idTable);
		E_ASSERT(lua_istable(L,-1));
		// other args
		lua_pushnumber(L, _pt.x);
		lua_pushnumber(L, _pt.y);
		// call
		int err = lua_pcall(L, 3, 1, 0);
		if(err)
		{
			kxxwin->L.RaiseLuaStackError();
			return false;
		}
		// return value
		E_ASSERT(lua_isboolean(L, -1));
		int ret = lua_toboolean(L, -1); 
		lua_pop(L, 1);
		return ret ? true : false;
	}

	void LuaShot::DropItem()
	{
		// NB_PROFILE_INCLUDE;
		if(idDropItem == LUA_REFNIL)
		{
			Shot::DropItem();
			return;
		}
		// function name
		lua_rawgeti(L, LUA_REGISTRYINDEX, idDropItem);
		E_ASSERT(lua_isfunction(L,-1));
		// push self
		lua_rawgeti(L, LUA_REGISTRYINDEX, idTable);
		E_ASSERT(lua_istable(L,-1));
		// call
		int err = lua_pcall(L, 1, 0, 0);
		if(err)
		{
			kxxwin->L.RaiseLuaStackError();
			return;
		}
	}

	bool LuaShot::Step()
	{
		// NB_PROFILE_INCLUDE;
		if(idStep == LUA_REFNIL)
		{
			return Shot::Step();
		}
		// function name
		lua_rawgeti(L, LUA_REGISTRYINDEX, idStep);
		E_ASSERT(lua_isfunction(L,-1));
		// push self
		lua_rawgeti(L, LUA_REGISTRYINDEX, idTable);
		E_ASSERT(lua_istable(L,-1));
		// call
		int err = lua_pcall(L, 1, 1, 0);
		if(err)
		{
			kxxwin->L.RaiseLuaStackError();
			return false;
		}
		// return value
		E_ASSERT(lua_isboolean(L, -1));
		int ret = lua_toboolean(L, -1); 
		lua_pop(L, 1);
		return ret ? true : false;
	}

	void LuaShot::Render()
	{
		// NB_PROFILE_INCLUDE;
		if(idRender == LUA_REFNIL)
		{
			Shot::Render();
			return;
		}
		// function name
		lua_rawgeti(L, LUA_REGISTRYINDEX, idRender);
		E_ASSERT(lua_isfunction(L,-1));
		// push self
		lua_rawgeti(L, LUA_REGISTRYINDEX, idTable);
		E_ASSERT(lua_istable(L,-1));
		// call
		int err = lua_pcall(L, 1, 0, 0);
		if(err)
		{
			kxxwin->L.RaiseLuaStackError();
			return;
		}
	}

	//bool LuaShot::RegisterForLua()
	//{

	////	int r;
	//	if(0 > script->RegisterInterface("ShotInterface"))
	//	{
	//		return false;
	//	}
	//	if(0 > script->RegisterInterfaceMethod("ShotInterface", "bool Collide(float, float, float)"))
	//	{
	//		return false;
	//	}
	//	if(0 > script->RegisterInterfaceMethod("ShotInterface", "bool OnHit(float, float)"))
	//	{
	//		return false;
	//	} 
	//	if(0 > script->RegisterInterfaceMethod("ShotInterface", "void DropItem()"))
	//	{
	//		return false;
	//	}
	//	if(0 > script->RegisterInterfaceMethod("ShotInterface", "bool Step()"))
	//	{
	//		return false;
	//	}
	//	if(0 > script->RegisterInterfaceMethod("ShotInterface", "void Render()"))
	//	{
	//		return false;
	//	}

	//	asIObjectType * shotInterface = script->GetObjectTypeById(script->GetTypeIdByDecl("ShotInterface"));

	//	idCollisionTest = shotInterface->GetMethodIdByName("Collide");
	//	idOnHit = shotInterface->GetMethodIdByName("OnHit");
	//	idDropItem = shotInterface->GetMethodIdByName("DropItem");
	//	idStep = shotInterface->GetMethodIdByName("Step");
	//	idRender = shotInterface->GetMethodIdByName("Render");

	//	return true;
	//}
#endif // E_CFG_LUA

}
