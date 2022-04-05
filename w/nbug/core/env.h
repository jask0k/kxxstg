
#ifndef E_ENV_H
#define E_ENV_H

#include <nbug/core/str.h>
#include <nbug/core/path.h>

namespace e
{
	struct ProcessorInfo
	{
		uint processorCount;
//		bool supportSSE2;
//		bool supportSSE3;
	};

	struct Env
	{
		static void SetShortName(const string & _name);
		static string GetShortName();
		static void SetDataFolder(const Path & _folder);
		static Path GetDataFolder();
		static void SetResourceFolder(const Path & _folder);
		static Path GetResourceFolder();
		static Path GetDocumentFolder();
		static Path GetExcuteFilePath();
		static Path GetHomeFolder();
		static stringa GetLocale();
		static string GetUserName();
		static void GetProcessorInfo(ProcessorInfo & _info);
		static string GetOSName();
	};
}

#endif
