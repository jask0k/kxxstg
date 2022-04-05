
#pragma once

#include <z_kxx/globals.h>
#include <nbug/tl/list.h>
#include <nbug/gl/graphics.h>

namespace e
{
	class RenderList;
	class Sprite
	{
		friend class e::RenderList;
		int      rlist_layer;
	public:
		Vector2   pos; // center point
		Vector2   hsz; // half size
		Vector2   scl; // scale
		float     ang; // rotate angle
		TexRef    tex; // texture
		BlendMode blm; 
		RGBA      clr;

		Sprite * rlist_prev;
		Sprite * rlist_next;

		Sprite * master;
		Sprite * pets;
		Sprite * pet_prev;
		Sprite * pet_next;

		Sprite * followed;
		Sprite * followers;
		Sprite * follower_prev;
		Sprite * follower_next;

		int hue;

		Sprite();
		virtual ~Sprite();
	
		void AddToRenderList(int _layer);
		void RemoveFromRenderList();
		virtual void Render();
		virtual void RenderDebug();
		virtual bool Step();
		int GetRenderListLayer() const;
		float Left() const;
		float Right() const;
		float Top() const;
		float Bottom() const;
		void SetSize(float _w, float _h);
		void SetSize(float _w);
		void RotateEastToVectorDirection(const Vector2 & _v);
		void RotateWestToVectorDirection(const Vector2 & _v);
		void RotateNorthToVectorDirection(const Vector2 & _v);
		void RotateSouthToVectorDirection(const Vector2 & _v);		
		void AddPet(Sprite * _p);
		void RemovePet(Sprite * _p);
		virtual void OnMasterDestroyed();
		void AddFollower(Sprite * _p);
		void RemoveFollower(Sprite * _p);
		virtual void OnFollowedDestroyed();
#ifdef E_CFG_LUA
		static int Lua_Pos(lua_State * L);
		static int Lua_Hsz(lua_State * L);
		static int Lua_Scl(lua_State * L);
		static int Lua_Ang(lua_State * L);
		static int Lua_Blm(lua_State * L);
		static int Lua_Clr(lua_State * L);
		static int Lua_Tex(lua_State * L);
#endif
	};

	typedef List<Sprite*> SpriteList;
}
