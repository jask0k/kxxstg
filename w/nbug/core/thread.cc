
//#include "private.h"
// #include "Exception.h"
#ifdef NB_WINDOWS
#	include <windows.h>
#	include <process.h>
#endif

#ifdef NB_LINUX
#	include <signal.h>
#	include <unistd.h>
#	include <pthread.h>
#	include <semaphore.h>
#	include <errno.h>
#endif
#include <nbug/core/thread.h>
#include <nbug/core/debug.h>
#include <nbug/core/def.h>

#undef malloc
#undef free

namespace e
{
	//Mutex global_mutex;

#ifdef NB_WINDOWS
#	define E_THREAD_RET uint __stdcall
#	define E_EXIT_THREAD ExitThread
	//DWORD g_main_thread_id = 0;
#endif	

#ifdef NB_LINUX
#	define E_THREAD_RET void *
#	define E_EXIT_THREAD pthread_exit
	//pthread_t g_main_thread_id = 0;
#endif

	struct Thread_o
	{
		//string name;
		void * param;	
		Callback entry;
#ifdef NB_WINDOWS
		void * native; //HANDLE
		//uint id;
#endif

#ifdef NB_LINUX
		unsigned long int native; // pthread_t
#endif
	};

	static E_THREAD_RET e_thread_launcher(void * _p)
	{
		Thread_o * thread_o = static_cast<Thread_o *>(_p);
		E_ASSERT(thread_o);
		thread_o->entry(thread_o->param);
		E_EXIT_THREAD(0);
	}

	Thread::Thread(Callback _entry, void * _param, uint _stackSize)
	{
		// use enew to triggle debug memory pool's initializion.
		thread_o = enew Thread_o; 
		//E_ASSERT(_entry.IsValid());
		thread_o->entry = _entry;
		thread_o->param = _param;

#ifdef NB_WINDOWS
		thread_o->native = (HANDLE) ::_beginthreadex(NULL,
			_stackSize,
			&e_thread_launcher,
			static_cast<void *>(thread_o),
			0 ,
			0);
		if(thread_o->native == NULL)
		{
			throw_posix(NB_SRC_LOC);
		}
/*
		switch(_priority)
		{
		case Low:
			::SetThreadPriority(native, THREAD_PRIORITY_BELOW_NORMAL);
			break;
		case Normal:
			::SetThreadPriority(native, THREAD_PRIORITY_NORMAL);
			break;
		case Height:
			::SetThreadPriority(native, THREAD_PRIORITY_ABOVE_NORMAL);
			break;
		default:
			E_ASSERT(0);
		}
		*/
#endif

#ifdef NB_LINUX
		pthread_attr_t tattr;
		int ret;
		sched_param param;
		ret = pthread_attr_init (&tattr);
		if(ret != 0)
		{
			throw_posix(NB_SRC_LOC);
		}
		ret = pthread_attr_getschedparam (&tattr, &param);
		if(ret != 0)
		{
			throw_posix(NB_SRC_LOC);
		}

		if(_stackSize != 0)
		{
			ret = pthread_attr_setstacksize(&tattr, _stackSize);
			if(ret != 0)
			{
				throw_posix(NB_SRC_LOC);
			}
		}
		int policy;
		ret = pthread_attr_getschedpolicy(&tattr, &policy);
		if(ret != 0)
		{
			throw_posix(NB_SRC_LOC);
		}

/*
#	ifdef NB_DEBUG
		int min_priority = sched_get_priority_min(policy);
		int max_priority = sched_get_priority_max(policy);
#	endif

		switch(_priority)
		{
		case Low:
			param.sched_priority = 20;
			break;
		case Normal:
			param.sched_priority = 20;
			break;
		case Height:
			param.sched_priority = 20;
			break;
		default:
			E_ASSERT(0);
		}
*/
		ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE) ;
		if(ret != 0)
		{
			throw_posix(NB_SRC_LOC);
		}

		ret =  pthread_create((pthread_t*)&thread_o->native, &tattr, &e_thread_launcher, thread_o);
		if(ret != 0)
		{
			throw_posix(NB_SRC_LOC);
		}
#endif

		E_TRACE_LINE(L"[nb] Thread start : 0x" + stringa::format("%X", thread_o->native));

	}

	Thread::~Thread()
	{
		bool joinok = false;
#ifdef NB_WINDOWS
		switch(WaitForSingleObject(thread_o->native, INFINITE)) // may dead lock?
		{
		case WAIT_OBJECT_0:
			joinok = true;
			break;
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			joinok = false;
		}
#endif
#ifdef NB_LINUX
		void* status;
		joinok = pthread_join((pthread_t)thread_o->native, &status) == 0;
#endif
		if(joinok)
		{
			E_TRACE_LINE(L"[nb] Thread exited : 0x" + stringa::format("%X", thread_o->native));
		}
		else
		{
			message(L"[nb] (WW) Fail to join thread: 0x" + stringa::format("%X", thread_o->native));
		}

		delete thread_o;
	}

	void Thread::Cancel()
	{
#ifdef NB_WINDOWS
		::TerminateThread(thread_o->native, 1);
#endif

#ifdef NB_LINUX
		::pthread_cancel((pthread_t)thread_o->native);
#endif
	}

	//void Thread::SetName(const string & _name)
	//{
	//	{
	//		_thread_global_mutex.Lock();
	//		thread_o->name = _name;
	//		_thread_global_mutex.Unlock();
	//	}
	//	E_TRACE_LINE(L"[nb] Thread::SetName() : 0x" + stringa::format("%X", thread_o->id) + L" => \"" + thread_o->name + L"\"");
	//}

	//string Thread::GetName() const
	//{
	//	ScopeLock sl(_thread_global_mutex);
	//	return thread_o->name;
	//}

	void Sleep(uint _millisecond)
	{
		// NB_PROFILE_EXCLUDE;
#ifdef NB_WINDOWS
		::Sleep(_millisecond);
#endif

#ifdef NB_LINUX
		::usleep(1000 * (uint64)_millisecond);
#endif
	}

#ifdef NB_WINDOWS

	Mutex::Mutex()
	{
		native = malloc(sizeof(CRITICAL_SECTION));
		::InitializeCriticalSection(((CRITICAL_SECTION*)native));
	}
	Mutex::~Mutex()
	{
		::DeleteCriticalSection(((CRITICAL_SECTION*)native));
		free(native);
	};

	void Mutex::Lock()
	{
		::EnterCriticalSection(((CRITICAL_SECTION*)native));
	}
	void Mutex::Unlock()
	{
		::LeaveCriticalSection(((CRITICAL_SECTION*)native));
	}


	Event::Event()
	{
		o= (void*) ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if(o == 0)
		{
			throw_winapi(NB_SRC_LOC);
		}
	}

	Event::~Event()
	{
		::CloseHandle((HANDLE)o);
	}

	void Event::Wait(bool _reset)
	{
		::WaitForSingleObject((HANDLE)o, INFINITE);
		if(_reset)
		{
			Reset();
		}
	}

	void Event::Set()
	{
		if(!::SetEvent((HANDLE)o))
		{
			throw_winapi(NB_SRC_LOC);
		}
	}

	bool Event::IsSet() const
	{
		return ::WaitForSingleObject((HANDLE)o, 0) != WAIT_TIMEOUT;
	}

	//Event::operator bool() const
	//{
	//	return ::WaitForSingleObject((HANDLE)o, 0) != WAIT_TIMEOUT;
	//}

	void Event::Reset()
	{
		::ResetEvent((HANDLE)o);
	}


	Semaphore::Semaphore(long _init)
	{
		o = (void *)::CreateSemaphore(NULL, _init, 0x7fffffff, NULL);
		if(o == 0)
		{
			throw_winapi(NB_SRC_LOC);
		}
	}

	Semaphore::~Semaphore()
	{
		::CloseHandle((HANDLE)o);
	}

	bool Semaphore::Wait()
	{
		return ::WaitForSingleObject((HANDLE)o, INFINITE) != WAIT_TIMEOUT;
	}

	bool Semaphore::Get()
	{
		return ::WaitForSingleObject((HANDLE)o, 0) != WAIT_TIMEOUT;
	}

	bool Semaphore::Release()
	{
		return ::ReleaseSemaphore((HANDLE)o, 1, NULL) == TRUE;
	}
#endif

#ifdef NB_LINUX

/*	struct Mutex_o
	{
	public:
		pthread_mutex_t native; // typedef union pthread_mutex_t;
		pthread_mutexattr_t mta; // typedef union pthread_mutexattr_t;
	};
*/
	Mutex::Mutex()
	{
		native = malloc(sizeof(pthread_mutex_t));
		mta = malloc(sizeof(pthread_mutexattr_t));
		int rc = pthread_mutexattr_init(((pthread_mutexattr_t*)mta));
		if(rc != 0)
		{
			throw_posix(NB_SRC_LOC);
		}
		rc = pthread_mutexattr_settype(((pthread_mutexattr_t*)mta), PTHREAD_MUTEX_RECURSIVE);
		if(rc != 0)
		{
			throw_posix(NB_SRC_LOC);
		}
		rc = pthread_mutex_init(((pthread_mutex_t*)native), ((pthread_mutexattr_t*)mta));
		if(rc != 0)
		{
			throw_posix(NB_SRC_LOC);
		}
	}

	Mutex::~Mutex()
	{
		pthread_mutex_destroy(((pthread_mutex_t*)native));
		pthread_mutexattr_destroy(((pthread_mutexattr_t*)mta));
		free(mta);
		free(native);
	};

	void Mutex::Lock()
	{
		pthread_mutex_lock(((pthread_mutex_t*)native));
	}
	void Mutex::Unlock()
	{
		pthread_mutex_unlock(((pthread_mutex_t*)native));
	}

	struct Event_o
	{
	public:
		pthread_mutex_t mutex;
		pthread_cond_t condition;
		bool isSet;
	};

	Event::Event()
	{
		o = (Event_o*) malloc(sizeof(Event_o));
		pthread_mutex_init(&((Event_o*)o)->mutex, NULL);
		pthread_cond_init(&((Event_o*)o)->condition, NULL);
		((Event_o*)o)->isSet = false;
	}

	Event::~Event()
	{
		pthread_mutex_destroy(&((Event_o*)o)->mutex);
		pthread_cond_destroy(&((Event_o*)o)->condition);
		free(o);
	}

	void Event::Wait(bool _reset)
	{
		pthread_mutex_lock(&((Event_o*)o)->mutex);
		while(!((Event_o*)o)->isSet)
		{
			int rc;
			rc = pthread_cond_wait(&((Event_o*)o)->condition, &((Event_o*)o)->mutex);
			if(rc && errno != EINTR )
			{
				break;
			}
		}
		((Event_o*)o)->isSet= _reset ? false : true;
		pthread_mutex_unlock(&((Event_o*)o)->mutex);
	}
	void Event::Set()
	{
		pthread_mutex_lock(&((Event_o*)o)->mutex);
		((Event_o*)o)->isSet=true;
		pthread_mutex_unlock(&((Event_o*)o)->mutex);
		pthread_cond_signal(&((Event_o*)o)->condition);
	}

	bool Event::IsSet() const
	{
		bool ret;
		pthread_mutex_lock(&((Event_o*)o)->mutex);
		ret = ((Event_o*)o)->isSet;
		pthread_mutex_unlock(&((Event_o*)o)->mutex);
		return ret;
	}

	void Event::Reset()
	{
		pthread_mutex_lock(&((Event_o*)o)->mutex);
		((Event_o*)o)->isSet = false;
		pthread_mutex_unlock(&((Event_o*)o)->mutex);
		//pthread_cond_signal(&((Event_o*)o)->condition);
	}

	Semaphore::Semaphore(long _init)
	{
		o = malloc(sizeof(sem_t)); // sem_t is a union
		int rc = sem_init((sem_t*)o, 0, _init);
		if(rc != 0)
		{
			// TODO: exception handling for Semaphore
		}
	}

	Semaphore::~Semaphore()
	{
		sem_destroy((sem_t*)o);
		free(o);
	}

	bool Semaphore::Wait()
	{
		return 0 == sem_wait((sem_t*)o);
	}

	bool Semaphore::Get()
	{
		int rc = sem_trywait((sem_t*)o);
		return rc == 0;
	}

	bool Semaphore::Release()
	{
		return 0 == sem_post((sem_t*)o);
	}
#endif

	ScopeLock::ScopeLock(Mutex & _mutex)
		: mutex(_mutex)
	{
		mutex.Lock();
	}
	ScopeLock::~ScopeLock()
	{
		mutex.Unlock();
	}


#ifdef E_CFG_UNIT_TEST
	static int TestThreadFunc(void * _p)
	{
		e::Sleep(rand() % 100);
		return 0;
	}

	E_UNIT_TEST_CASE(MultiThread)
	{
		const int THREAD_COUNT = 10;
		e::Thread * t[THREAD_COUNT] ;
		for(int i=0; i< THREAD_COUNT; i++)
		{
			t[i] = enew e::Thread(&TestThreadFunc, 0);
			//t[i]->SetName(L"Test thead " + string(i));
		}
		// e::Sleep(1000);
		for(int i=0; i< THREAD_COUNT; i++)
		{
			//t[i]->WaitExit();
			delete t[i];
		}
		return true;
	}
#endif
}

