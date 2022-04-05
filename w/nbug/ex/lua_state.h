
#pragma once


#ifdef E_CFG_LUA

extern "C" 
{
#	include <lua.h>
#	include <lualib.h>
#	include <lauxlib.h>
}

//#	ifdef NB_DEBUG
//#		pragma comment(lib, "debug_liblua")
//#	else
//#		pragma comment(lib, "liblua")
//#	endif

#include <nbug/core/file.h>
#include <nbug/core/callback.h>
namespace e
{
	class LuaState
	{
		lua_State * L;
		static int _OnPanic(lua_State * L); // error outside protected environment
		//int OnPanic();
		//static int _OnError(lua_State * L); // error inside protected environment
		//void OnError(const string & _s);
		string lastError;
		Callback errorCallback;
		int RaiseError(const string & _s);
		bool _Load(FileRef _file, const stringa & _chuckName);
	public:
		int RaiseLuaStackError();
		LuaState();
		~LuaState();
		operator lua_State*() const
		{ return L; }
		lua_State * operator->()
		{ return L; }
		bool Load(const Path & _path);
		//bool Load(FileRef _file, const stringa & _chuckName);
		//bool Load(DirectoryRef _dir);
		//void GetGlobal(const char * _name);
		//int CallGlobal(const char * _name, int _nargs = 0, int _nresults = 0);
		int Call(int _nargs = 0, int _nresults = 0);
		static LuaState * GetLuaState(lua_State * L);
		string GetLastError() const
		{ return lastError; }
		Callback SetErrorCallback(const Callback & _cb);
		void Close();
	};


#define	lua_tofloat(L, n) ((float)lua_tonumber((L),(n)))
#define E_LUA_ERROR(L, s) luaL_error((L), "Error in C func:\n\t%s(%d): %s", __FILE__, __LINE__, (s));

}

#endif

