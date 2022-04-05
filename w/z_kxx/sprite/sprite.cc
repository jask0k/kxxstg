
// #include "../config.h"
#include <z_kxx/sprite/sprite.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/util/render_list.h>

namespace e
{

	bool Sprite::Step()
	{
		return true;
	}

	void Sprite::RemoveFromRenderList()
	{
		if(rlist_layer>= 0)
		{
			rlist->Remove(this);
		}
	}

	Sprite::~Sprite()
	{
		if(followed)
		{
			followed->RemoveFollower(this);
		}

		if(followers)
		{
			Sprite * p = followers;
			while(p)
			{
				Sprite * p1 = p;
				p = p->follower_next;
				p1->followed = 0;
				p1->OnFollowedDestroyed();
			}
		}


		if(master)
		{
			master->RemovePet(this);
		}

		if(pets)
		{
			Sprite * p = pets;
			while(p)
			{
				Sprite * p1 = p;
				p = p->pet_next;
				p1->master = 0;
				p1->OnMasterDestroyed();
			}
		}

		if(rlist_layer>= 0)
		{
			rlist->Remove(this);
		}
	}

	void Sprite::OnMasterDestroyed()
	{}

	void Sprite::OnFollowedDestroyed()
	{}

	void Sprite::AddToRenderList(int _layer)
	{
		if(rlist_layer>=0)
		{
			rlist->Remove(this);
		}
		rlist->Add(this, _layer);
	}

	void Sprite::RenderDebug()
	{
	}

	void Sprite::Render()
	{
		if(!tex)
		{
			return;
		}

		graphics->SetColor(clr);
		graphics->SetBlendMode(blm);

		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);
		if(ang)
		{
			graphics->RotateMatrix(ang, 0, 0, 1);
		}
		float scaledHalfWidth = hsz.x * scl.x;
		float scaledHalfHeight = hsz.y * scl.y;
		graphics->BindTex(tex);
		graphics->DrawQuad(-scaledHalfWidth, -scaledHalfHeight, scaledHalfWidth, scaledHalfHeight);
		graphics->PopMatrix();
	}

	Sprite::Sprite()
	{
		pos.x = 0;
		pos.y = 0;
		scl.x = 1.0f;
		scl.y = 1.0f;
		hsz.x = 100;
		hsz.y = 100;
		ang = 0;
		//alpha = 1.0f;
		blm = BM_NORMAL;
		clr.r = clr.g = clr.b = clr.a = 1.0f;
		rlist_layer = -1;
		master = 0;
		pets = 0;
		followed = 0;
		followers = 0;
		hue = 0;
	}

	int Sprite::GetRenderListLayer() const
	{ 
		return rlist_layer; 
	}

	float Sprite::Left() const
	{
		return pos.x - hsz.x * scl.x;
	}

	float Sprite::Right() const
	{
		return pos.x + hsz.x * scl.x;
	}

	float Sprite::Top() const
	{
		return pos.y - hsz.y * scl.y;
	}

	float Sprite::Bottom() const
	{
		return pos.y + hsz.y * scl.y;
	}

	void Sprite::SetSize(float _w, float _h)
	{
		hsz.x = _w * 0.5f;
		hsz.y = _h * 0.5f;
	}

	void Sprite::SetSize(float _w)
	{
		hsz.x = _w * 0.5f;
		hsz.y = hsz.x;
	}

	void Sprite::RotateEastToVectorDirection(const Vector2 & _v)
	{
		ang = _v.Angle();;
	}

	void Sprite::RotateWestToVectorDirection(const Vector2 & _v)
	{
		ang = _v.Angle() + PI;
	}

	void Sprite::RotateNorthToVectorDirection(const Vector2 & _v)
	{
		ang = _v.Angle() + (PI * 0.5f);
	}

	void Sprite::RotateSouthToVectorDirection(const Vector2 & _v)
	{
		ang = _v.Angle() - (PI * 0.5f);
	}
		
	void Sprite::AddPet(Sprite * _p)
	{
		E_ASSERT(_p->master == 0);
		if(pets)
		{
			_p->pet_next = pets;
			_p->pet_next->pet_prev = _p;
			_p->pet_prev = 0;
			pets = _p;
		}
		else
		{
			_p->pet_prev = 0;
			_p->pet_next = 0;
			pets = _p;
		}
		_p->master = this;
	}

	void Sprite::RemovePet(Sprite * _p)
	{
		E_ASSERT(_p->master == this);
		if(_p->pet_next)
		{
			_p->pet_next->pet_prev = _p->pet_prev;
		}
		if(_p->pet_prev)
		{
			_p->pet_prev->pet_next = _p->pet_next;
		}
		else
		{
			E_ASSERT(pets == _p);
			pets = _p->pet_next;
		}
		_p->master = 0;
	}

	void Sprite::AddFollower(Sprite * _p)
	{
		E_ASSERT(_p->followed == 0);
		if(followers)
		{
			_p->follower_next = followers;
			_p->follower_next->follower_prev = _p;
			_p->follower_prev = 0;
			followers = _p;
		}
		else
		{
			_p->follower_prev = 0;
			_p->follower_next = 0;
			followers = _p;
		}
		_p->followed = this;
	}

	void Sprite::RemoveFollower(Sprite * _p)
	{
		E_ASSERT(_p->followed == this);
		if(_p->follower_next)
		{
			_p->follower_next->follower_prev = _p->follower_prev;
		}
		if(_p->follower_prev)
		{
			_p->follower_prev->follower_next = _p->follower_next;
		}
		else
		{
			E_ASSERT(followers == _p);
			followers = _p->follower_next;
		}
		_p->followed = 0;
	}



#ifdef E_CFG_LUA
	static Sprite * LuaToSprite(lua_State * L, int _index)
	{
		int type = lua_type(L, _index);
		if(type == LUA_TUSERDATA)
		{
			void *ud = lua_touserdata (L, _index);
			if(ud == 0)
			{
				//luaL_typerror(L, _index, CLASSNAME);
				E_ASSERT(0);
			}
			return *(Sprite**)ud;
		}
		else if(type != LUA_TNIL)
		{
			//luaL_typerror(L, _index, CLASSNAME);
			E_ASSERT(0);
		}

		return 0;
	}


	int Sprite::Lua_Pos(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			lua_pushnumber(L, p->pos.x);
			lua_pushnumber(L, p->pos.y);
			return 2;
		}
		else if(top == 3)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->pos.x = lua_tofloat(L, 2);		
			p->pos.y = lua_tofloat(L, 3);		
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}

	int Sprite::Lua_Hsz(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			lua_pushnumber(L, p->hsz.x);
			lua_pushnumber(L, p->hsz.y);
			return 2;
		}
		else if(top == 3)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->hsz.x = lua_tofloat(L, 2);		
			p->hsz.y = lua_tofloat(L, 3);		
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}

	int Sprite::Lua_Scl(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			lua_pushnumber(L, p->scl.x);
			lua_pushnumber(L, p->scl.y);
			return 2;
		}
		else if(top == 3)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->scl.x = lua_tofloat(L, 2);		
			p->scl.y = lua_tofloat(L, 3);		
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}

	int Sprite::Lua_Ang(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			lua_pushnumber(L, p->ang);
			return 1;
		}
		else if(top == 2)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->ang = lua_tofloat(L, 2);		
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}

	int Sprite::Lua_Blm(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			lua_tointeger(L, p->blm);
			return 1;
		}
		else if(top == 2)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->blm = (BlendMode)lua_tointeger(L, 2);		
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}

	int Sprite::Lua_Clr(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			lua_pushnumber(L, p->clr.r);
			lua_pushnumber(L, p->clr.g);
			lua_pushnumber(L, p->clr.b);
			lua_pushnumber(L, p->clr.a);
			return 4;
		}
		else if(top == 4 || top == 5)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->clr.r = lua_tofloat(L, 2);		
			p->clr.g = lua_tofloat(L, 3);		
			p->clr.b = lua_tofloat(L, 4);		
			p->clr.a = top == 5 ? lua_tofloat(L, 5) : 1;
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}

	int Sprite::Lua_Tex(lua_State * L)
	{
		int top = lua_gettop(L);
		if(top == 1)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			Tex::Lua_PushTex(L, p->tex.ptr());
			return 1;
		}
		else if(top == 2)
		{
			Sprite * p = LuaToSprite(L, 1);
			E_ASSERT(p);
			p->tex = Tex::Lua_ToTex(L, 2);
			return 0;
		}
		else
		{
			E_ASSERT(0);
			return 0;
		}
	}
#endif
}

