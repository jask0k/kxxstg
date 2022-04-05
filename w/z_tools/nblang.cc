#include <nbug/core/file.h>
#include <nbug/core/ini.h>

#include <nbug/tl/set.h>
#include <string.h>


using namespace e;

Path srcRoot;
Path dstFile;

void WriteLine(const stringa & _text)
{
	E_TRACE(_text);
	puts(_text.c_str());
}

void ParseLine(const stringa & _line, e::Set<string> & _set)
{
	//if(_line.substr(0, 30) == "box.SetMessage(_path.GetText()")
	//{
	//	E_DEBUG_BREAK;
	//}
	const char * src = _line.c_str();
	while(true)
	{
		const char * pp = strstr(src, "_TT(\"");
		if(pp == NULL)
		{
			return;
		}

		const char * p0 =  pp + 5;
		const char * p = p0;
		//if(p[0] == 'C' && p[1] == 'h')
		//{
		//	E_DEBUG_BREAK;
		//}

		stringa t;
		while(true)
		{
			if(*p == '\\')
			{
				if(*(p+1))
				{
					t.append(*(p+1));
					p+= 2;
				}
				else
				{
					return;
				}
			}
			else if(*p== '\"' && *(p+1) == ')')
			{
				p+= 2;
				src = p;
				break;
			}
			else if(*p)
			{
				t.append(*p);
				p++;
			}
			else
			{
				return;
			}
		}
		// TODO: optimize
		t.replace("&&", "[:amp:]");
		t.replace("&", "");
		t.replace("[:amp:]", "&");
		_set.insert(string(t));
	}
}

void ParseFile(FileRef _file, e::Set<string> & _set)
{
	//WriteLine(_TT("Parsing FileRef") + L": " + _path.GetText());
	stringa line;
	while(_file->ReadLine(line))
	{
		ParseLine(line, _set);
	}
}


int main(int _argc, char ** _argv)
{
	if(_argc != 3)
	{
		WriteLine(_TT("Usage") + L": ");
		WriteLine(L"        nblang src_folder dst_file");
		return 1;
	}

	srcRoot = string(_argv[1]);
	dstFile = string(_argv[2]);

	if(srcRoot.IsRelative())
	{
		srcRoot = FS::GetCurrentFolder() | srcRoot;
	}

	if(dstFile.IsRelative())
	{
		dstFile = FS::GetCurrentFolder() | dstFile;
	}

	if(!FS::IsFolder(srcRoot))
	{
		WriteLine(_TT("Invalid src folder") + L": " + string(_argv[1]));
		return 1;
	}

	if(FS::IsFolder(dstFile))
	{
		WriteLine(_TT("Invalid dst file") + L": " + string(_argv[2]));
		return 1;
	}


	WriteLine(_TT("Parsing Folder") + L": " + srcRoot.GetString());

	string fileName;
	DirectoryRef dir = FS::OpenDir(srcRoot);
	if(!dir)
	{
		WriteLine("ERROR: Fail to read src_dir.");
		return 1;
	}


	e::Set<string> set0;

	do
	{
		//if(dir->IsDir())
		//{
		//	continue;
		//}

		fileName = dir->GetName();
		if(MatchFileName(fileName, "*.cpp") || MatchFileName(fileName, "*.h"))
		{
			WriteLine(_TT("Parsing FileRef") + L": " + (srcRoot | dir->GetRelativePath()).GetString());
			FileRef file = dir->OpenFile(true);
			ParseFile(file, set0);
		}
	}while(dir->MoveNext(true));


	IniFile ini;
	ini.Load(dstFile);
	StringArray keys = ini.GetAllKey();

	for(size_t i=0; i<keys.size(); i++)
	{
		const string & k = keys[i];
		if(set0.find(k) == set0.end())
		{
			set0.insert(k);
			string value;
			ini.Get(k, value);
			if(value.length() < 5 || value.substr(0, 5) != L"<DEL>")
			{
				ini.Set(k, L"<DEL>" + value);
			}
		}
	}

	string tTemp;
	for(e::Set<string>::iterator it = set0.begin(); it != set0.end(); ++it)
	{
		//all.push_back(*it);
		if(!ini.Get(*it, tTemp))
		{
			ini.Set(*it, *it);
		}
	}


	if(ini.empty())
	{
		FS::Delete(dstFile);
	}
	else
	{
		//FS::Touch(dstFile);
		ini.Save(dstFile);
	}

	return 0;
}
