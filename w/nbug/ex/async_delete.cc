#include <nbug/ex/async_delete.h>
#include <nbug/tl/list.h>
#include <nbug/core/thread.h>
#include <nbug/core/time.h>

namespace e
{
	List<IAsyncDelete*> g_async_delete_list;
	static bool g_once_try_delete = false;
	Mutex mutex;
	static void AsyncDeleteAtExit()
	{
		if(!g_once_try_delete && !g_async_delete_list.empty())
		{
			message(L"[nb] AsyncDeleteAtExit(): nerver try delete, but list is not empty.");
		}
		AsyncDeleteRoutine(true);
	}

	void AsyncDelete(IAsyncDelete * _p)
	{
		mutex.Lock();
		static bool _once = false;
		if(!_once)
		{
			_once = true;
			atexit(&AsyncDeleteAtExit);
		}
		g_async_delete_list.push_back(_p);
		mutex.Unlock();
	}

	void AsyncDeleteRoutine(bool _block)
	{
		mutex.Lock();
		g_once_try_delete = true;
		do
		{
			List<IAsyncDelete*>::iterator it = g_async_delete_list.begin();
			while(it != g_async_delete_list.end())
			{
				IAsyncDelete * _p = *it;
				bool deleted = _p->OnAsyncDelete();
				if(deleted)
				{
					it = g_async_delete_list.erase(it);
				}
				else
				{
					++it;
				}
			}
		}while(_block && !g_async_delete_list.empty());
		mutex.Unlock();
	}
}
