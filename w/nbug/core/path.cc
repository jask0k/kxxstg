
//#include "private.h"
#include <nbug/tl/array.h>
#include <nbug/core/path.h>
#include <nbug/core/str.h>
#include <nbug/core/debug.h>
// #include "Exception.h"

namespace e
{
	const Path Path::InvalidPath;

#ifdef NB_WINDOWS
#	define E_FILEPATH_HAVE_DRIVER
	static const Char * _stdSeparator = L"/";
	static const Char * _invalidPartChars = L"\\/:*?\"<>|\n\t\r";
	static const Char * _separators = L"\\/";
#endif

#ifdef NB_LINUX
#	define E_FILEPATH_CASE_SENSITIVE
	static const Char * _stdSeparator = L"/";
	static const Char * _invalidPartChars = L"\\/:*?\"<>|";
	static const Char * _separators = L"/";
#endif

	static inline bool _IsPartEqual(const string & _part1, const string & _part2)
	{
		// NB_PROFILE_INCLUDE;
#ifdef E_FILEPATH_CASE_SENSITIVE
		return _part1 == _part2;
#else
		return _part1.icompare(_part2) == 0;
#endif
	}

	static inline bool _IsPartLess(const string & _part1, const string & _part2)
	{
		// NB_PROFILE_INCLUDE;
#ifdef E_FILEPATH_CASE_SENSITIVE
		return _part1 < _part2;
#else
		return _part1.icompare(_part2) < 0;
#endif
	}

	static bool _IsValidName(const string & _part)
	{
		// NB_PROFILE_INCLUDE;
		return _part.length() > 0 && _part.find_any(_invalidPartChars) == -1;
	}

	static bool _IsDot(const string & _part)
	{
		// NB_PROFILE_INCLUDE;
		return _part == string(L".");
	}
	static bool _IsDotDot(const string & _part)
	{
		// NB_PROFILE_INCLUDE;
		return _part == string(L"..");
	}
#ifdef E_FILEPATH_HAVE_DRIVER
	static bool _IsValidDriverName(Char _ch)
	{
		// NB_PROFILE_INCLUDE;
		return _ch >= L'A' && _ch <= L'Z' ||
			_ch >= L'a' && _ch <= L'z';
	}
#endif

	Path::Path(const string & _path) 
	{
		this->isAbsolute = false;
		this->isValid = false;
		this->driver = 0;

		int len = _path.length();
		int pos = 0;
		if(len >=8 && _path.substr(0, 8).icompare(L"file:///") == 0)
		{
			pos = 8;
		}

#ifdef E_FILEPATH_HAVE_DRIVER
		if(pos + 2 <= len)
		{
			if(_path[pos + 1] == L':')
			{
				if(!_IsValidDriverName(_path[pos]))
				{
					//E_THROW(L"Bad this->driver: " + string(_path[pos]));
					//throw (e);
					E_ASSERT(0);
				}
				this->driver = _path[pos];
				pos+= 2;
			}
		}
#endif
		if(pos < len)
		{
			//Array<Char> _separators;

			// determine if is absolute path
			this->isAbsolute = _path.find_any(_separators, pos) == pos;
			StringArray v;
			Split(_path, _separators, pos, -1).swap(v);
			//StringArray::iterator it;
			//for(it = v.begin(); it != v.end(); ++it)
			for(uint i=0; i < v.size(); i++)
			{
				string & t = v[i];
				// E_TRACE_LINE(t);
				if(_IsDot(t))
				{
					continue;
				}
				else if(_IsDotDot(t))
				{
					UpOneLevel();
				}
				else if(_IsValidName(t))
				{
					this->parts.push_back(t);
				}
				else if(!t.empty())
				{
					E_ASSERT1(0, L"Invalid string: \"" + t + L"\" in " + _path);
					this->isValid = false;
					this->parts.clear();
				}
			}
		}

		this->isValid = true;
#ifdef NB_DEBUG
		debugText = GetString();
#endif
	}

	Path::Path(int zero)
	{
		// NB_PROFILE_INCLUDE;
		this->isAbsolute = false;
		this->isValid = false;
		this->driver = 0;
	}
	
	Path::Path(const Path & _r) 
	{
		//o = enew Path_o;
		this->isAbsolute = _r.isAbsolute;
		this->isValid = _r.isValid;
		this->parts = _r.parts;
		this->driver=_r.driver;

#ifdef NB_DEBUG
		debugText = _r.debugText;
#endif
	}

	const Path & Path::operator=(const Path & _r)
	{
		// NB_PROFILE_INCLUDE;
		if(this != &_r)
		{
			this->isAbsolute = _r.isAbsolute;
			this->isValid = _r.isValid;
			this->parts = _r.parts;
#ifdef E_FILEPATH_HAVE_DRIVER
			this->driver=_r.driver;
#endif

#ifdef NB_DEBUG
			debugText = _r.debugText;
#endif
		}
		return *this;
	}


	bool Path::IsValid() const
	{
		// NB_PROFILE_INCLUDE;
		return this->isValid;
	}

	bool Path::operator==(const Path & _r) const
	{
		// NB_PROFILE_INCLUDE;
		if (IsAbsolute() != _r.IsAbsolute()
#ifdef E_FILEPATH_HAVE_DRIVER
			|| this->driver != _r.driver
#endif
			|| this->parts.size() != _r.parts.size())
		{
			return false;
		}

		for(uint i = 0; i < parts.size(); i++)
		{
			if(!_IsPartEqual(parts[i], _r.parts[i]))
			{
				return false;
			}
		}

		return true;
	}

	bool Path::operator!=(const Path & _r) const
	{
		// NB_PROFILE_INCLUDE;
		return !operator==(_r);
	}

	bool Path::operator<(const Path & _r) const
	{
		// NB_PROFILE_INCLUDE;
		if(IsAbsolute() != _r.IsAbsolute())
		{
			return !IsAbsolute();
		}

#ifdef E_FILEPATH_HAVE_DRIVER
		if(driver != _r.driver)
		{
			return driver < _r.driver;
		}
#endif
		if(parts.size() != _r.parts.size())
		{
			return parts.size() < _r.parts.size();
		}

		for(uint i = 0; i < parts.size(); i++)
		{
			if(_IsPartLess(parts[i], _r.parts[i]))
			{
				return true;
			}
		}

		return false;
	}

	bool Path::IsAbsolute() const
	{
		// NB_PROFILE_INCLUDE;

#ifdef E_FILEPATH_HAVE_DRIVER
		return this->driver != 0 || this->isAbsolute;
#else
		return this->isAbsolute;
#endif
	}

	bool Path::IsRelative() const
	{
		// NB_PROFILE_INCLUDE;
		return !IsAbsolute();
	}

	Path Path::operator|(const Path & _r) const
	{
		// NB_PROFILE_INCLUDE;
		Path ret(*this);
		ret.Join(_r);
		return ret;
	}

	Path Path::operator|(const string & _r) const
	{
		Path ret(*this);
		ret.Join(Path(_r));
		return ret;
	}

	void Path::Join(const Path & _r)
	{
		// NB_PROFILE_INCLUDE;
		if(!this->isValid || !_r.isValid)
		{
			this->isValid = false;
			this->parts.clear();
#ifdef NB_DEBUG
			debugText = GetString();
#endif
			return;
		}

		if(_r.IsAbsolute())
		{
			//E_THROW(L"Join with abslute path.");
			//throw(e);
			E_ASSERT(0);
		}
		//Parts::const_iterator it;
		//for(it = _r.parts.begin(); it != _r.parts.end(); ++it)
		for(uint i = 0; i < _r.parts.size(); i++)
		{
			const string & t = _r.parts[i];
			E_ASSERT(!_IsDot(t));
			E_ASSERT(_IsValidName(t));
			if(_IsDotDot(t))
			{
				UpOneLevel();
			}
			else
			{
				this->parts.push_back(t);
			}
		}

#ifdef NB_DEBUG
		debugText = GetString();
#endif
	}

	stringa Path::GetStringA() const
	{
		// NB_PROFILE_INCLUDE;
		return GetString();
	}

	string Path::GetString() const
	{
		// NB_PROFILE_INCLUDE;
		string ret;
		if(!IsValid())
		{
			ret = L"<*INVALID PATH*>";
		}

#ifdef E_FILEPATH_HAVE_DRIVER
		if(this->driver != 0)
		{
			//if(addProtocolPrefix)
			//{
			//	ret+=L"/";
			//}
			ret += string(this->driver) + L":";
		}
#endif
		if(IsAbsolute())
		{
			ret+= _stdSeparator;
		}
		bool havePrev = false;
//		Parts::const_iterator it;
//		for(it = this->parts.begin(); it != this->parts.end(); ++it)
		for(uint i = 0; i < this->parts.size(); i++)
		{
		//	E_TRACE_LINE(*it);
			if(havePrev)
			{
				ret+= _stdSeparator;
			}
			else
			{
				havePrev = true;
			}
			ret+= this->parts[i];
		}
		
		return ret;
	}

	Path Path::operator-(const Path & _r) const
	{
		// NB_PROFILE_INCLUDE;
		// 
		if(_r.IsRelative() || this->IsRelative())
		{
			//E_THROW(L"Get relative path require two absolute path.");
			//throw(e);
			E_ASSERT(0);
		}

		//if(_r == *this)
		//{
		//	return L".";
		//}

#ifdef E_FILEPATH_HAVE_DRIVER
		// 
		// 
		Char c1 = _r.driver;
		Char c2 = this->driver;
		if(c1 >= 'A' && c1 <='Z')
		{
			c1 = c1 + 'a' - 'A';
		}
		if(c2 >= 'A' && c2 <='Z')
		{
			c2 = c2 + 'a' - 'A';
		}
		if(_r.driver != 0 && this->driver != 0 && c1 != c2 )
		{
			E_ASSERT(0);
		}
#endif
		const StringArray & parts2 = this->parts;
		size_t minSize = _r.parts.size() < parts2.size() ? _r.parts.size() : parts2.size();
		// find the fork point
		// /a/b/c/1/2/3
		// +++++++-------
		// /a/b/c/d/e/f/g

		size_t fork;
		for(fork = 0; fork < minSize; fork++)
		{
#ifdef E_FILEPATH_CASE_SENSITIVE
			if(_r.parts[fork] != parts2[fork])
				break;
#else
			if(_r.parts[fork].icompare(parts2[fork]) != 0)
				break;
#endif
		}

		Path ret;
		// back to fork point
		for(uint i = fork; i < _r.parts.size(); i++)
		{
			ret.parts.push_back(L"..");
		}

		// swith to base directory
		for(uint i = fork; i < parts2.size(); i++)
		{
			ret.parts.push_back(parts2[i]);
		}
#ifdef NB_DEBUG
		ret.debugText = ret.GetString();
#endif
		ret.isValid = true;
		ret.isAbsolute = false;
		return ret;
	}
//	Path Path::GetAbsolute(const Path & _baseOn) const
//	{
//		
//		if(IsAbsolute())
//		{
//			//E_THROW(L"Get absolute path require relative path.");
//			//throw(e);
//			E_ASSERT(0);
//		}
//
//		
//		if(_baseOn.IsRelative())
//		{
//			//E_THROW(L"Get absolute path must base checked absolute path.");
//			//throw(e);
//			E_ASSERT(0);
//		}
//
//		Path ret(_baseOn);
////		Parts::const_iterator it;
////		for(it = this->parts.begin(); it != this->parts.end(); ++it)
//		for(int i=0; i< this->parts.size(); i++)
//		{
//			const string & t = this->parts[i];
//			E_ASSERT(!_IsDot(t));
//			E_ASSERT(!t.empty());
//			if(_IsDotDot(t))
//			{
//				ret.UpOneLevel();
//			}
//			else
//			{
//				ret.parts.push_back(t);
//			}
//		}
//E_DEBUG_ONLY(ret.debugText = ret.GetString();)
//		return ret;
//	}

	string Path::GetBaseName(bool _with_ext) const
	{
		// NB_PROFILE_INCLUDE;
		if(this->parts.empty())
		{
			return L"";
		}
		else if(_with_ext)
		{
			return this->parts.back();
		}
		else
		{
			const string & s = this->parts.back();
			int pos = s.rfind(L'.');
			if(pos == -1)
			{
				return s;
			}
			else
			{
				return s.substr(0, pos);
			}
		}
	}

	string Path::GetExtension() const
	{
		if(this->parts.empty())
		{
			return L"";
		}
		else
		{
			const string & s = this->parts.back();
			int pos = s.rfind(L'.');
			if(pos == -1)
			{
				return L"";
			}
			else
			{
				return s.substr(pos + 1, s.length() - pos - 1);
			}
		}
	}

	bool Path::HasParentFolder() const
	{
		return isValid && !parts.empty();
	}

	Path Path::GetParentFolder() const
	{
		// NB_PROFILE_INCLUDE;
		Path ret(*this);
		if(HasParentFolder())
		{
			ret.parts.pop_back();
		}
		else
		{
			if(IsAbsolute())
			{
				throw(NB_SRC_LOC "Up one level of root.");
			}
			ret.parts.push_back(L"..");
		}
#ifdef NB_DEBUG
		ret.debugText = ret.GetString();
#endif
		return ret;
	}

	bool Path::AppendToLastPart(const string & _ext)
	{
		if(this->parts.empty())
		{
			return false;
		}
		this->parts.back().append(_ext);
#ifdef NB_DEBUG
		debugText = GetString();
#endif
		return true;
	}

	bool Path::RemoveLastChar()
	{
		if(this->parts.empty())
		{
			return false;
		}
		string & s = this->parts.back();
		int len = s.length();
		if(len < 2)
		{
			return false;
		}

		s[len-1] = 0;
#ifdef NB_DEBUG
		debugText = GetString();
#endif
		return true;
	}

	void Path::UpOneLevel()
	{
		// NB_PROFILE_INCLUDE;
		if(HasParentFolder())
		{
			this->parts.pop_back();
		}
		else
		{
			if(IsAbsolute())
			{
				throw(NB_SRC_LOC "Up one level of root.");
			}
			this->parts.push_back(L"..");
		}
#ifdef NB_DEBUG
		debugText = GetString();
#endif
	}

	StringArray Path::GetParts() const
	{
		// NB_PROFILE_INCLUDE;
		return StringArray(this->parts);
	}

	bool Path::CheckExtension(const string & _ext) const
	{
		// NB_PROFILE_INCLUDE;
		if(!this->isValid || this->parts.empty())
		{
			return false;
		}
		//const string & fn = this->parts.back();
		return GetExtension().icompare(_ext) == 0;
	}

	void Path::ReplaceExtension(const string & _new_ext)
	{
		if(isValid && !parts.empty())
		{
			string & s = parts.back();
			int pos = s.rfind(L'.');
			if(pos == -1)
			{
				s.append(L"." + _new_ext);
			}
			else
			{
				s = s.substr(0, pos) +  L"." + _new_ext;
			}
#ifdef NB_DEBUG
			debugText = GetString();
#endif
		}
	}

	void Path::RemoveExtension()
	{
		if(isValid && !parts.empty())
		{
			string & s = parts.back();
			int pos = s.rfind(L'.');
			if(pos != -1)
			{
				s = s.substr(0, pos);
			}
#ifdef NB_DEBUG
			debugText = GetString();
#endif
		}
	}

	void Path::AddExtension(const string & _ext)
	{
		if(isValid && !parts.empty())
		{
			string & s = parts.back();
			s.append(L"." + _ext);
#ifdef NB_DEBUG
			debugText = GetString();
#endif
		}
	}

}
