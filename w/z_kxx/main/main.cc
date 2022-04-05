
// #include "../config.h"
#include <string>
#ifdef NB_WINDOWS
#	include <windows.h>
#endif
#include <nbug/core/time.h>
#include <nbug/core/env.h>
#include <nbug/core/file.h>
#include <nbug/core/translate.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/main/options.h>
#include <nbug/ex/async_delete.h>


#if !defined(KXX_INSTALL_PREFIX) && defined(NB_LINUX)
#	define KXX_INSTALL_PREFIX "/usr/local"
#endif

#define K_LINUX_RES_FOLDER KXX_INSTALL_PREFIX "/share/kxx"

#define KXX_RUNNING_LOCK_FILE (Env::GetDataFolder() | L"running.lock")


namespace e
{
	extern double g_startup_time;
	extern const char * GetCompiledDate_yymmdd();
	static bool consoleQuit = false;


	void ShowConfigDialog(bool _crashed);

	void CheckAndShowConfigDialog()
	{
		bool needShow = false;
		bool crashed = true;
		bool lastQuitIsProper = !FS::IsFile(KXX_RUNNING_LOCK_FILE);
		if(!lastQuitIsProper)
		{
	//		message(L"[kx] (WW) Seems program didn't close properly. Show the config dialog now.");
			needShow = true;
			crashed = true;
		}
		else if(!FS::IsFile(Env::GetDataFolder() | L"options.ini"))
		{
	//		message(L"[kx] Seems is first run. Show the config dialog now.");
			needShow = true;
		}

		if(needShow)
		{
			ShowConfigDialog(crashed);
		}
	}

	#ifdef NB_WINDOWS
	BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
	{
		message(L"[kx] (WW) Recieved console event, force close.");
		consoleQuit = true;
		return TRUE;
	}

	BOOL WINAPI MyEnumWndProc(HWND hWnd,LPARAM lParam)
	{
		HANDLE h = ::GetProp(hWnd, K_WIN_UNIQUE_PROP);
		if(h == (HANDLE)1)
		{
			*((HWND*)lParam) = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	bool FindAndActiveOldInstance()
	{
		HWND h = 0;
		::EnumWindows(&MyEnumWndProc, (LPARAM) &h);
		if(h!=0)
		{
			message(L"[kx] One instance of this program is running.");
			string s = _TT("INSTANCE_RUNNING0") + L"\n" + _TT("INSTANCE_RUNNING1");
			string t = _TT("KXX01");
			::MessageBox(NULL, s.c_str(), t.c_str(), MB_OK);
			::PostMessage(h, WM_USER+105, 0, 0);
			return true;
		}
		return false;
	}

	#endif


	int my_main()
	{
		g_startup_time = Time::GetTicks();
#	ifdef NB_WINDOWS
		SetConsoleTitle(L"KXX DEBUG OUTPUT");
		SetConsoleCtrlHandler(&HandlerRoutine, TRUE);
#	endif

		Env::SetShortName(L"kxx");

		Path exeFolder = Env::GetExcuteFilePath().GetParentFolder();

	#ifdef NB_DEBUG
		static const bool isPortableMode = true;
	#else
		bool isPortableMode = FS::IsFile(exeFolder | L"portable");
	#endif
		// E_TRACE_LINE(L"[kx] isPortableMode = " + string(isPortableMode));

		if(isPortableMode)
		{
			Env::SetDataFolder(exeFolder| L"data");
			Env::SetResourceFolder(exeFolder | L"res");
			//Env::SetResourceFolder(string("D:/w/z_kxx/res"));
		}
		else
		{
	#ifdef NB_LINUX
			Env::SetResourceFolder(Path(string(K_LINUX_RES_FOLDER)));
	#else
			Env::SetResourceFolder(exeFolder | L"res");
	#endif
		}


		message(L"");
		message(L"[kx] ===== KXX(" + string(GetCompiledDate_yymmdd()) + L") start. " + Time::Current().GetString(L"%Y-%m-%d %H:%M:%S", false) + L" =====");

		string translationFile = Env::GetLocale() + L".txt";
		LoadTranslation(Env::GetResourceFolder() | L"translation" | translationFile);

	#ifdef NB_WINDOWS
		if(FindAndActiveOldInstance())
		{
			return 1;
		}
	#endif

	//	enew int;
		if(isPortableMode)
		{
			message(L"[kx] Run in portable mode");
		}
		else
		{
			message(L"[kx] Run in non-portable mode");
		}
		message(L"[kx] Resource folder: " + QuoteString(Env::GetResourceFolder().GetString()));
		message(L"[kx] Data folder: " + QuoteString(Env::GetDataFolder().GetString()));



	#ifdef NB_DEBUG
		{
			int n = 0;
			Path path;
			bool b = true;
			do
			{
				path = Env::GetDataFolder() | ( L"rnd_dump_" + string(n) + L".txt");
				n++;
				b = FS::IsFile(path);
				if(b)
				{
					FS::Delete(path);
				}
			}while(b || n < 10);
			ProcessorInfo info;
			Env::GetProcessorInfo(info);
		}
	#endif

		//while(true)
		//{
		//	message(L"[kx] (WW) Failed to load dictionary: ");
		//}
		//exit(0);

		double d = e::Time::GetTicks();
		d-= floor(d);
		uint32 seed = uint32(d * RAND_MAX);
		srand(seed);

		if(!is_debugger_present())
		{
			double t0 = Time::GetTicks();
			CheckAndShowConfigDialog();
			g_startup_time+= Time::GetTicks() - t0;
		}

		if(!FS::IsFile(KXX_RUNNING_LOCK_FILE))
		{
			FS::OpenFile(KXX_RUNNING_LOCK_FILE, true);
		}

		// e::report_error("adsfsdaf\"dsafsdaf");
		KxxOptions * kxxOptions = enew KxxOptions();
		kxxOptions->Load();

	#ifdef NB_WINDOWS
		if(kxxOptions->graphicsBackend == 1)
		{
			Graphics::SetDeviceType(Graphics::DirectX);
		}
	#endif

	//	GenerateSpeedTable();
	//	E_ASSERT(0);
		kxxwin = enew KxxWin(kxxOptions);
		if(!kxxwin->Create())
		{
			return 1;
		}

		LoopState loopState;
		while(!consoleQuit && (loopState = Win::PumpMessage(0.02)) != LoopQuit && kxxwin != 0)
		{
			kxxwin->OnRealTime(loopState == LoopBusy);
			if(loopState != LoopBusy)
			{
				AsyncDeleteRoutine(false);
			}
		}
		if(kxxwin)
		{
			delete kxxwin;
		}

		if(FS::IsFile(KXX_RUNNING_LOCK_FILE))
		{
			FS::Delete(KXX_RUNNING_LOCK_FILE);
		}

		return 0;
	}

}

using namespace e;
int main(int _argc, char ** _argv)
{
	int ret = 1;
	try
	{
		ret = my_main();
	}
	catch(const char * _exp)
	{
		error(_exp);
		ret = 1;
	}
	catch(...)
	{
		error(NB_SRC_LOC "unhandled exception raised.");
		ret = 1;
	}

	message(L"[kx] ===== KXX(" + string(GetCompiledDate_yymmdd()) + L") exit. " + Time::Current().GetString(L"%Y-%m-%d %H:%M:%S", false) + L" =====");
	message(L"");	
	return ret;
}

