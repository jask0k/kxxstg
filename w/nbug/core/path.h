#ifndef NB_FILE_PATH_H
#define NB_FILE_PATH_H

#include <nbug/core/str.h>
#include <nbug/tl/str_array.h>

namespace e
{
	// file path on local file system
	//  "/a/./../b" will be converted to "/b".
	// "./a/../../b" will be converted to  "../b".
	class Path
	{
#ifdef NB_DEBUG
		string debugText;
#endif
		StringArray parts;
		bool isAbsolute : 1;
		bool isValid : 1;
		Char driver; // always 0 on Linux
	public:
		Path(int _zero = 0);
		Path(const Path & _r);
		Path(const string & _path);
		bool operator==(const Path & _r) const;
		bool operator!=(const Path & _r) const;
		bool operator<(const Path & _r) const;
		const Path & operator=(const Path & _r);
		bool IsAbsolute() const;
		bool IsRelative() const;
		bool IsValid() const;
		Path operator|(const Path & _r) const;
		Path operator|(const string & _r) const;
		void Join(const Path & _r);
		string GetString() const;
		stringa GetStringA() const; // useful for use system api under linux
		Path operator-(const Path & _r) const;
		string GetBaseName(bool _with_ext = true) const;
		string GetExtension() const;
		bool CheckExtension(const string & _ext) const;
		void ReplaceExtension(const string & _new_ext);
		void RemoveExtension();
		void AddExtension(const string & _ext);
		void UpOneLevel();
		bool HasParentFolder() const;
		Path GetParentFolder() const;
		StringArray GetParts() const;
		static const Path InvalidPath;
		bool AppendToLastPart(const string & _ext);
		bool RemoveLastChar();
	};
}

#endif
