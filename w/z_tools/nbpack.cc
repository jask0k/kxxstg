#include <nbug/core/str.h>
#include <nbug/core/file.h>
//#include <nbug/core/thread.h>
#include <nbug/tl/list.h>
using namespace e;

void WriteLine(const stringa & _text)
{
	E_TRACE(_text);
	puts(_text.c_str());
}

void PrintUsage()
{
	WriteLine("Usage:");
	WriteLine("nbpack.exe dst_file src_dir");
}

int my_main(int _argc, char ** _argv)
{
	List<string> exclude;

	if(_argc < 3)
	{
		PrintUsage();
		return 1;
	}

	if(_argc > 4)
	{
		string s = _argv[3];
		if(s.icompare(L"-E") == 0)
		{
			for(int i=4; i<_argc; i++)
			{
				exclude.push_back(_argv[i]);
				//WriteLine("exclude: \"" + stringa(_argv[i]) + "\"" );
			}
		}
	}

	Path src = string(_argv[2]);
	if(src.IsRelative())
	{
		src = FS::GetCurrentFolder() | src;
	}

	Path dst = string(_argv[1]);
	if(dst.IsRelative())
	{
		dst = FS::GetCurrentFolder() | dst;
	}

	DirectoryRef it = FS::OpenDir(src);
	if(!it)
	{
		WriteLine("ERROR: Fail to read src_dir: \"" + stringa(_argv[2]) + "\"" );
		PrintUsage();
		return 1;
	}

	FileRef dstFile = FS::OpenFile(dst, true);
	if(!dstFile)
	{
		WriteLine("ERROR: Fail to crate dst_file: \"" + stringa(_argv[1]) + "\"" );
		PrintUsage();
		return 1;
	}
	dstFile->SetSize(0);
	if(!CreateFilePack(dstFile, it, exclude))
	{
		WriteLine("ERROR: An error occur while packing files, aborted.");
		return 1;
	}

	WriteLine("succeeded.");
	return 0;
}

int main(int _argc, char ** _argv)
{
	try
	{
		return my_main(_argc, _argv);
	}
	catch(...)
	{
		E_ASSERT(0);
		WriteLine("failed.");
		return 1;
	}
}
