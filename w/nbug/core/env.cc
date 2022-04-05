
//#include "private.h"

#ifdef NB_WINDOWS
#	include <shlobj.h> // shell32.lib
#endif

#ifdef NB_LINUX
#	include <pwd.h>
#	include <unistd.h>
#	include <locale.h>
#	include <string.h>
#endif

#ifndef MAX_PATH
#	ifdef NB_LINUX
#		include <limits.h>
#		define MAX_PATH PATH_MAX
#	endif
#endif

#include <nbug/core/env.h>
#include <nbug/core/debug.h>
#include <nbug/core/path.h>
#include <nbug/core/file.h>
#include <nbug/core/ini.h>

namespace e
{
	static bool g_short_name_explicit_set = false;
	string g_explicit_set_short_name;
	static bool g_data_folder_explicit_set = false;
	Path g_explicit_set_data_folder;
	static bool g_resource_folder_explicit_set = false;
	Path g_explicit_set_resource_folder;

	void Env::SetShortName(const string & _name)
	{
		g_short_name_explicit_set = true;
		g_explicit_set_short_name = _name;
	}

	void Env::SetDataFolder(const Path & _folder)
	{
		g_data_folder_explicit_set = true;
		g_explicit_set_data_folder = FS::ParseShortcutFile(_folder);
	}

	void Env::SetResourceFolder(const Path & _folder)
	{
		g_resource_folder_explicit_set = true;
		g_explicit_set_resource_folder = FS::ParseShortcutFile(_folder);
	}


	Path Env::GetResourceFolder()
	{
		return g_resource_folder_explicit_set 
			? g_explicit_set_resource_folder
			: Env::GetExcuteFilePath().GetParentFolder();
	}

	string Env::GetShortName()
	{
		return g_short_name_explicit_set ? g_explicit_set_short_name : GetExcuteFilePath().GetBaseName(false);
	}


	static Path GetRootDataFolder()
	{
#ifdef E_DATA_DIR_IN_EXE_DIR
		return Env::GetExcuteFilePath().GetParentFolder();
#else

#	ifdef NB_WINDOWS
		Char buf[MAX_PATH];
		if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_APPDATA, 0, 0, buf)))
		{
			return Path(buf);
		}
		else
		{
			return Env::GetExcuteFilePath().GetParentFolder() | L"data";
		}
#	endif
#	ifdef NB_LINUX
		uid_t uid;
		struct passwd * pw;
		uid = ::getuid();
		pw = ::getpwuid(uid);
		return Path(pw->pw_dir);
#	endif
#endif
	}

	Path Env::GetDataFolder()
	{
		if(g_data_folder_explicit_set)
		{
			return g_explicit_set_data_folder;
		}
		else
		{
#ifdef NB_WINDOWS
		return GetRootDataFolder() | Env::GetShortName();
#endif
#ifdef NB_LINUX
		return GetRootDataFolder() | (string(".") + GetShortName());
#endif
		}
	}

	Path Env::GetExcuteFilePath()
	{
#ifdef NB_WINDOWS
		TCHAR buf[MAX_PATH];
		if(GetModuleFileName(NULL, buf, MAX_PATH) == 0)
		{
			return Path::InvalidPath;
		}
		return Path(buf);
#endif

#ifdef NB_LINUX
		char buf[MAX_PATH];
		int count = readlink("/proc/self/exe", buf, MAX_PATH);
		if(count == -1)
		{
			return Path();
		}
		buf[count] = 0;
		return Path(buf);
#endif
	}


	Path Env::GetDocumentFolder()
	{
		//E_STACK_TRACE;


#ifdef NB_WINDOWS
		Char buf[MAX_PATH] = {0};
		if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_PERSONAL, 0, 0, buf)))
		{
			return Path(buf);
		}
		else
		{
			return Path::InvalidPath;
		}

#endif
#ifdef NB_LINUX
		Path ret = GetHomeFolder() | string("Documents"); // TODO: read from xdg
		if(FS::IsFolder(ret))
		{
			//E_TRACE_LINE(L"UserDocumentFolder = " + ret.GetString());
			return ret;
		}
		else
		{
			return GetHomeFolder();
		}
#endif
	}


	Path Env::GetHomeFolder()
	{
#ifdef NB_WINDOWS
		Char buf[MAX_PATH] = {0};
		if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_PROFILE, 0, 0, buf)))
		{
			return Path(buf);
		}
		else
		{
			return Path::InvalidPath;
		}

#endif
#ifdef NB_LINUX
		uid_t uid;
		struct passwd * pw;
		uid = ::getuid();
		pw = ::getpwuid(uid); // just ignore memory leaks warning, it's harmless.
		return Path(pw->pw_dir);
#endif
	}

//	Path Env::GetDesktopFolder()
//	{
//#ifdef NB_WINDOWS
//		Char buf[MAX_PATH] = {0};
//		if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_PROFILE, 0, 0, buf)))
//		{
//			return Path(buf);
//		}
//		else
//		{
//			return Path::InvalidPath;
//		}
//
//#endif
//#ifdef NB_LINUX
//		uid_t uid;
//		struct passwd * pw;
//		uid = ::getuid();
//		pw = ::getpwuid(uid); // just ignore memory leaks warning, it's harmless.
//		return Path(pw->pw_dir) | L"Desktop";
//#endif
//	}


	stringa Env::GetLocale()
	{
		stringa _locale;
#ifdef NB_LINUX
		//setlocale(LC_CTYPE, "");
		//char * charset = nl_langinfo(CODESET);
		//if(charset)
		//{
		//	setlocale(LC_CTYPE, charset);
		//}
		//else
		//{
		//	setlocale(LC_CTYPE, "C");
		//}
		message(L"[nb] (WW) GetLocale() still unimplemented under Linux");
		_locale = "zh_CN";
#endif

#ifdef NB_WINDOWS
		LCID  id = GetSystemDefaultLCID();
		char buf[256];
		GetLocaleInfoA(id, LOCALE_SISO639LANGNAME   , buf, 256);
		_locale = buf;
		GetLocaleInfoA(id, LOCALE_SISO3166CTRYNAME  , buf, 256);
		if(strlen(buf) > 0)
		{
			_locale+= stringa("_") + buf;
		}
#endif
		message(L"[nb] Locale = \"" + _locale + L"\"");
		return _locale;
	}

	string Env::GetUserName()
	{
#ifdef NB_WINDOWS
		DWORD n = 1024;
		Char buf[1024];
		if(::GetUserName(buf, &n))
		{
			return buf;
		}
		else
		{
			return L"<unkown>";
		}
#endif

#ifdef NB_LINUX
		uid_t uid;
		struct passwd * pw;
		uid = ::getuid();
		pw = ::getpwuid(uid);
		return string(pw->pw_name);
#endif

	}

	void Env::GetProcessorInfo(ProcessorInfo & _info)
	{
		static bool _loaded = false;
		static ProcessorInfo _savedInfo;
		if(_loaded)
		{
			memcpy(&_info, &_savedInfo, sizeof(_info));
			return;
		}

		memset(&_savedInfo, 0, sizeof(_savedInfo));
		// TODO: use asm?
#ifdef NB_LINUX
		FileRef file = FS::OpenFile(Path(L"/proc/cpuinfo"), false);
		if(file)
		{
			stringa t;
			while(file->ReadLine(t))
			{
				if(t.find_str("processor") == 0)
				{
					_savedInfo.processorCount++;
				}
//				else if(t.length() > 5 && memcmp("flags", &t[0], 5) == 0)
//				{
//					_savedInfo.supportSSE2 = t.find_str("sse2") != -1;
				//	_savedInfo.supportSSE3 = t.find_str("sse3") != -1; // TODO: ssse3 vs sse3
//				}
			}
		}
	
#endif

#ifdef NB_WINDOWS
		SYSTEM_INFO info;
		::GetSystemInfo(&info);
		_savedInfo.processorCount = (uint)info.dwNumberOfProcessors;

//		_savedInfo.supportSSE2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) ? true : false;
		//_savedInfo.supportSSE3 = IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) ? true : false;

#endif

		if(_savedInfo.processorCount == 0)
		{
			_savedInfo.processorCount = 1;
		}

		message(L"[nb] Processor info : processor count = " + string(_savedInfo.processorCount));
//		E_TRACE_LINE(L"[nb] Processor info : support SSE2 = " + string(_savedInfo.supportSSE2));
		memcpy(&_info, &_savedInfo, sizeof(_info));
		_loaded = true;
	}

	string Env::GetOSName()
	{
#ifdef NB_WINDOWS
		string ret = L"Microsoft Windows";
		SYSTEM_INFO si;
		OSVERSIONINFOEX os;
		os.dwOSVersionInfoSize=sizeof(os);
		if(GetVersionEx((OSVERSIONINFO*)&os))
		{
#	ifdef __GNUC__

			if(os.dwMajorVersion == 6 && os.dwMinorVersion == 1)
			{
				ret+= L" 7";
			}
  			else if(os.dwMajorVersion == 6 && os.dwMinorVersion == 0)
			{
				ret+= L" Vista";
			}
  			else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 1)
			{
				ret+= L" XP";
			}
  			else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 0)
			{
				ret+= L" 2000";
			}
			else
			{
				ret+= L" " + string((int)os.dwMajorVersion) + L"." + string((int)os.dwMinorVersion);
			}

#	else
			GetNativeSystemInfo(&si);
			bool is64 = os.dwPlatformId == VER_PLATFORM_WIN32_NT 
				&& si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
			switch(os.dwPlatformId)
			{
			case VER_PLATFORM_WIN32_NT:
				if(os.dwMajorVersion == 6 && os.dwMinorVersion == 1 && os.wProductType == VER_NT_WORKSTATION)
				{
					ret+= L" 7";
				}
				else if(os.dwMajorVersion == 6 && os.dwMinorVersion == 1 && os.wProductType != VER_NT_WORKSTATION)
				{
					ret+= L" Server 2008 R2";
				}
				else if(os.dwMajorVersion == 6 && os.dwMinorVersion == 0 && os.wProductType == VER_NT_WORKSTATION)
				{
					ret+= L" Vista";
				}
				else if(os.dwMajorVersion == 6 && os.dwMinorVersion == 0 && os.wProductType != VER_NT_WORKSTATION)
				{
					ret+= L" Server 2008";
				}
				else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) != 0)
				{
					ret+= L" Server 2003 R2";
				}
				else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 2 && os.wSuiteMask & VER_SUITE_WH_SERVER)
				{
					ret+= L" Home Server";
				}
				else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) == 0)
				{
					ret+= L" Server 2003";
				}
				else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 2 && (os.wProductType == VER_NT_WORKSTATION) && (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64))
				{
					ret+= L" Professional x64 Edition";
				}
				else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 1)
				{
					ret+= L" XP";
				}
  				else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 0)
				{
					ret+= L" 2000";
				}
				else
				{
					ret+= L" " + string((int)os.dwMajorVersion) + L"." + string((int)os.dwMinorVersion);
				}
				break;
			default:
				ret+= L" " + string((int)os.dwMajorVersion) + L"." + string((int)os.dwMinorVersion);
				break;
			}

			if(os.szCSDVersion[0])
			{
				ret+= L" " + string(os.szCSDVersion);
			}
#endif
		}
		return ret;
#else
		return L"Linux";
#endif
	}

}
 
