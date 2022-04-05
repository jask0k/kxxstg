
#ifndef NB_CORE_THREAD_H
#define NB_CORE_THREAD_H

#include <nbug/core/obj.h>
#include <nbug/core/str.h>
#include <nbug/core/callback.h>

namespace e
{
	class Mutex
	{
		void * native;
#ifdef NB_LINUX
		void * mta; // for recursive
#endif	
	public:
		Mutex();
		~Mutex();
		void Lock();
		void Unlock();
	};

	//extern Mutex global_mutex;

	class ScopeLock
	{
		Mutex & mutex;
	public:
		ScopeLock(Mutex & _mutex);
		~ScopeLock();
	};

	class Event
	{
		void * o;
	public:
		Event();
		~Event();
		void Wait(bool _reset = true);
		void Set();
		bool IsSet() const;
		void Reset();
	};

	class Semaphore
	{
		void * o;
	public:
		Semaphore(long _init = 0);
		~Semaphore();
		bool Wait();
		bool Get();
		bool Release();
	};

	void Sleep(uint _millisecond);
	struct Thread_o;
	class Thread
	{
		Thread_o * thread_o;
	public:
		Thread(Callback _entry, void * _param, uint _stackSize = 0);
		~Thread();
		//void SetName(const string & _name);
		//string GetName() const;
		void Cancel();
	};

	// bool CreateThread(const Callback & _entry, void * _param);
}

#endif

