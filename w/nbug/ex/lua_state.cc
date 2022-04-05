
//#include "private.h"
#ifdef E_CFG_LUA

#include <nbug/core/debug.h>
//#include "MemoryPool.h"
#include "LuaState.h"

namespace e
{

	
	typedef e::Map<lua_State *, LuaState *> LuaStateMap;
	static LuaStateMap g_luaStateMap;
	LuaState * LuaState::GetLuaState(lua_State * L)
	{
		LuaStateMap::iterator it = g_luaStateMap.find(L);
		return it == g_luaStateMap.end() ? 0 : it->second;
	}

	int LuaState::_OnPanic(lua_State * L)
	{
		LuaState * p = GetLuaState(L);
		E_ASSERT(p);
		if(p)
		{
			p->RaiseLuaStackError();
		}
		return 0;
	}

	
	/*int LuaState::_OnError(lua_State * L)
	{
		lua_Debug d;
		lua_getstack(L, 1, &d);
		lua_getinfo(L, "Sln", &d);
		stringa s0 = lua_tostring(L, -1);
		int n = s0.find_str("]:");
		if(n != -1)
		{
			n = s0.find(':', n+2);
			if(n != -1)
			{
				s0 = s0.substr(n + 1, -1);
			}
		}
		lua_pop(L, 1);

		stringa s = stringa(d.source) + "(" + stringa(d.currentline) + "):"  
			+ s0;		
		
		LuaState * p = GetLuaState(L);
		E_ASSERT(p);
		if(p)
		{
			p->OnError(s);
		}

		lua_pushstring(L, s.c_str());
		return 1;
	}*/

	//void LuaState::OnError(const string & _s)	
	//{
	//	lastError = _s;
	//	E_TRACE_LINE(lastError);
	//}

	//int LuaState::CallGlobal(const char * _name, int _nargs, int _nresults)
	//{
	//	lua_pushcclosure(L, &_OnError, 0);
	//	lua_getglobal(L, _name);
	//	return lua_pcall(L, _nargs, _nresults, -2);
	//}

	static void * LuaAllocFunc (void * _ud, void * _ptr, size_t _osize, size_t _nsize)
	{
		(void)_ud;  (void)_osize;
		if(_nsize == 0) 
		{
			//MPoolFree(_ptr);
			free(_ptr);
			return NULL;
		}
		else if(_ptr == 0)
		{
//#ifdef NB_DEBUG
//			return MPoolAlloc(_nsize, __FILE__, __LINE__);
//#else
//			return MPoolAlloc(_nsize);
//#endif
			return malloc(_nsize);
		}
		else
		{
//#ifdef NB_DEBUG
//			E_TRACE_LINE(L"MPoolRealloc() _ptr = " + PointerToHex(_ptr) + L", _nsize=" + string(_nsize));
//			return MPoolRealloc(_ptr, _nsize, __FILE__, __LINE__);
//#else
//			return MPoolRealloc(_ptr, _nsize);
//#endif
			return realloc(_ptr, _nsize);
		}
	}
	
	//static void * LuaAllocFunc (void * _ud, void * _ptr, size_t _osize, size_t _nsize)
	//{
	//	(void)_ud;  (void)_osize;
	//	if(_nsize == 0) 
	//	{
	//		free(_ptr);
	//		return NULL;
	//	}
	//	else if(_ptr == 0)
	//	{
	//		return malloc(_nsize);
	//	}
	//	else
	//	{
	//		return realloc(_ptr, _nsize);
	//	}
	//}

	LuaState::LuaState()
	{
		E_TRACE_LINE("[nb] LuaState::LuaState()");
		L = lua_newstate(&LuaAllocFunc, 0);
		lua_atpanic(L, &LuaState::_OnPanic);
		g_luaStateMap[L] = this;
	}

	void LuaState::Close()
	{
		if(L)
		{
			//lua_gc(L, LUA_GCCOLLECT, 0);
			g_luaStateMap.erase(L);
			lua_close(L);
			L = 0;
		}
	}

	LuaState::~LuaState()
	{
		E_TRACE_LINE("[nb] LuaState::~LuaState()");
		Close();
	}


	struct MyLuaReader
	{
		FileRef file;
		char buf[1024];
		static const char * Reader(lua_State * _L, void * _data, size_t * _size)
		{
			MyLuaReader * _this = (MyLuaReader *) _data;
			size_t & sz = *_size;
			sz = 0;
			sz = _this->file->ReadSome(_this->buf, 1024);
			return _this->buf;
		}
	};

	bool LuaState::_Load(FileRef _file, const stringa & _chuckName)
	{
		if(!_file)
		{
			RaiseError(L"Fail to load lua script(s): " + _chuckName);
			return false;
		}

		int err;
		{
			MyLuaReader * reader = enew MyLuaReader;
			reader->file = _file;
			err = lua_load(L, &MyLuaReader::Reader, reader, _chuckName.c_str());
			delete reader;
		}
		_file.Detach();

		if(err)
		{
			RaiseLuaStackError();
			return false;
		}

		if(Call(0,LUA_MULTRET))
		{
			return false;
		}
		return true;
	}
	//	return true;
	//}
	int LuaState::RaiseError(const string & _s)
	{
		lastError = _s;
		return errorCallback(this);
	}

	bool LuaState::Load(const Path & _path)
	{
		E_TRACE_LINE(L"[nb] LuaState::Load(): _path = " + _path.GetString());
		if(FS::IsFolder(_path))
		{
			DirectoryRef dir = FS::OpenDir(_path);
			if(!dir)
			{
				RaiseError(L"Fail to load lua script(s): " + _path.GetString());
				return false;
			}

			bool loaded = false;
			string name;
			string pattern = "*.lua";
			do
			{
				if(dir->IsDir())
				{
					continue;
				}
				name = dir->GetName();
				if(!MatchFileName(name, pattern))
				{
					E_TRACE_LINE(L"    Ignore: " + name);
					continue;
				}

				E_TRACE_LINE(L"    Load: " + name);
				FileRef f = dir->OpenFile();
				if(_Load(f, dir->GetFullPath().GetStringA()))
				{
					loaded = true;
				}
				else
				{
					return false;
				}
				/*stringa fullName = dir->GetFullPath().GetStringA();
				if(!f)
				{
					dir.Detach();
					RaiseError(L"Fail to load lua script(s): " + fullName);
					return false;
				}

				int err;
				{
					MyLuaReader * reader = enew MyLuaReader;
					reader->file = f;
					err = lua_load(L, &MyLuaReader::Reader, reader, fullName.c_str());
					delete reader;
				}
				f.Detach();

				if(err)
				{
					dir.Detach();
					RaiseLuaStackError();
					return false;
				}

				if(Call(0,LUA_MULTRET))
				{
					dir.Detach();
					return false;
				}*/
					
			}while(dir->MoveNext(true));
			return loaded;

		}
		else if(FS::IsFile(_path))
		{
			FileRef f = FS::OpenFile(_path);
			stringa fullName = _path.GetStringA();
			return _Load(f, _path.GetStringA());
		}
		else
		{
			RaiseError(L"Fail to load lua script(s): " + _path.GetString());
			return false;
		}
	}

	//bool LuaState::Load(DirectoryRef _dir)
	//{
	//	bool loaded = false;
	//	string name;
	//	string pattern = "*.lua";
	//	do
	//	{
	//		if(!_dir->IsDir())
	//		{
	//			 name = _dir->GetName();
	//			 if(MatchFileName(name, pattern))
	//			 {
	//				 E_TRACE_LINE(L"[nb] Load script file: " + name);
	//				 FileRef f = _dir->OpenFile();
	//				 if(Load(f, _dir->GetFullPath().GetStringA()))
	//				 {
	//					 loaded = true;
	//				 }
	//				 else
	//				 {
	//					 return false;
	//				 }
	//			 }
	//		}
	//	}while(_dir->MoveNext(true));
	//	return loaded;
	//}

	int LuaState::RaiseLuaStackError()
	{
		string s = lua_tostring(L, -1);
		lua_pop(L, 1);
		int n0 = s.find_str(L"[string \"");
		int n1 = s.find_str(L"\"]", n0+9);
		int n2 = s.find(':', n1+1);
		int n3 = s.find(':', n2+1);

		if(n0 != -1 && n1 != -1 && n2 != -1 && n3 != -1)
		{
			string file = s.substr(n0+9, n1 - n0 - 9);
			string line = s.substr(n2+1, n3 - n2 - 1);
			string msg = s.substr(n3 + 1, -1);
			lastError = file + L"(" + line + L"):" + msg;
		}
		else
		{
			lastError = s;
		}

		return errorCallback(this);
	}

	int LuaState::Call(int _nargs, int _nresults)
	{
		int err =  lua_pcall(L, _nargs, _nresults, 0);
		if(err)
		{
			RaiseLuaStackError();
		}
		return err;
	}

	Callback LuaState::SetErrorCallback(const Callback & _cb)
	{
		Callback ret = errorCallback;
		errorCallback = _cb;
		return ret;
	}

}

#endif
